/**
 * @file ota_server.c
 * @brief Node HTTP OTA firmware upload server.
 */

#include "ota_server.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"

static const char *TAG = "OTA_SERVER";

#define OTA_URI_PATH       "/ota"
#define OTA_WRITE_BUFFER   4096U

static httpd_handle_t s_server = NULL;

/* ========================================================================== */
/* OTA POST handler                                                           */
/* ========================================================================== */

static esp_err_t ota_post_handler(httpd_req_t *req)
{
    if (req == NULL)
    {
        return ESP_ERR_INVALID_ARG;
    }

    if (req->content_len <= 0)
    {
        ESP_LOGE(
            TAG,
            "Invalid OTA content length: %d",
            req->content_len
        );

        httpd_resp_send_err(
            req,
            HTTPD_400_BAD_REQUEST,
            "Invalid firmware size"
        );

        return ESP_FAIL;
    }

    ESP_LOGI(
        TAG,
        "OTA upload started: %d bytes",
        req->content_len
    );

    const esp_partition_t *update_partition =
        esp_ota_get_next_update_partition(NULL);

    if (update_partition == NULL)
    {
        ESP_LOGE(
            TAG,
            "No OTA update partition available"
        );

        httpd_resp_send_err(
            req,
            HTTPD_500_INTERNAL_SERVER_ERROR,
            "No OTA partition"
        );

        return ESP_FAIL;
    }

    esp_ota_handle_t ota_handle = 0;

    esp_err_t err =
        esp_ota_begin(
            update_partition,
            (size_t)req->content_len,
            &ota_handle
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "esp_ota_begin failed: %s",
            esp_err_to_name(err)
        );

        httpd_resp_send_err(
            req,
            HTTPD_500_INTERNAL_SERVER_ERROR,
            "OTA begin failed"
        );

        return err;
    }

    uint8_t buffer[OTA_WRITE_BUFFER];
    int total_received = 0;

    while (total_received < req->content_len)
    {
        int remaining =
            req->content_len - total_received;

        int to_receive =
            remaining > (int)sizeof(buffer)
                ? (int)sizeof(buffer)
                : remaining;

        int received =
            httpd_req_recv(
                req,
                (char *)buffer,
                to_receive
            );

        if (received == HTTPD_SOCK_ERR_TIMEOUT)
        {
            ESP_LOGW(
                TAG,
                "OTA receive timeout"
            );

            continue;
        }

        if (received <= 0)
        {
            ESP_LOGE(
                TAG,
                "OTA receive failed: %d",
                received
            );

            (void)esp_ota_abort(ota_handle);

            httpd_resp_send_err(
                req,
                HTTPD_500_INTERNAL_SERVER_ERROR,
                "OTA receive failed"
            );

            return ESP_FAIL;
        }

        err =
            esp_ota_write(
                ota_handle,
                buffer,
                (size_t)received
            );

        if (err != ESP_OK)
        {
            ESP_LOGE(
                TAG,
                "esp_ota_write failed: %s",
                esp_err_to_name(err)
            );

            (void)esp_ota_abort(ota_handle);

            httpd_resp_send_err(
                req,
                HTTPD_500_INTERNAL_SERVER_ERROR,
                "OTA write failed"
            );

            return err;
        }

        total_received += received;

        ESP_LOGI(
            TAG,
            "OTA received: %d/%d bytes",
            total_received,
            req->content_len
        );
    }

    err = esp_ota_end(ota_handle);

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "esp_ota_end failed: %s",
            esp_err_to_name(err)
        );

        httpd_resp_send_err(
            req,
            HTTPD_500_INTERNAL_SERVER_ERROR,
            "OTA finalize failed"
        );

        return err;
    }

    err =
        esp_ota_set_boot_partition(
            update_partition
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to set boot partition: %s",
            esp_err_to_name(err)
        );

        httpd_resp_send_err(
            req,
            HTTPD_500_INTERNAL_SERVER_ERROR,
            "Boot partition failed"
        );

        return err;
    }

    ESP_LOGI(
        TAG,
        "OTA upload accepted: %d bytes",
        total_received
    );

    httpd_resp_set_type(
        req,
        "text/plain"
    );

    httpd_resp_send(
        req,
        "OTA OK",
        HTTPD_RESP_USE_STRLEN
    );

    ESP_LOGI(
        TAG,
        "Restarting Node"
    );

    vTaskDelay(pdMS_TO_TICKS(100U));

    esp_restart();

    return ESP_OK;
}

/* ========================================================================== */
/* Server lifecycle                                                           */
/* ========================================================================== */

NfwStatus_t otaServerStart(void)
{
    if (s_server != NULL)
    {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    httpd_config_t config =
        HTTPD_DEFAULT_CONFIG();

    config.server_port = 80;
    config.max_uri_handlers = 4;

    esp_err_t err =
        httpd_start(
            &s_server,
            &config
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "HTTP server start failed: %s",
            esp_err_to_name(err)
        );

        s_server = NULL;

        return NFW_STATUS_ERROR;
    }

    httpd_uri_t ota_uri = {
        .uri = OTA_URI_PATH,
        .method = HTTP_POST,
        .handler = ota_post_handler,
        .user_ctx = NULL
    };

    err =
        httpd_register_uri_handler(
            s_server,
            &ota_uri
        );

    if (err != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "Failed to register OTA URI: %s",
            esp_err_to_name(err)
        );

        (void)httpd_stop(s_server);
        s_server = NULL;

        return NFW_STATUS_ERROR;
    }

    ESP_LOGI(
        TAG,
        "OTA HTTP server started: POST %s",
        OTA_URI_PATH
    );

    return NFW_STATUS_OK;
}

NfwStatus_t otaServerStop(void)
{
    if (s_server == NULL)
    {
        return NFW_STATUS_NOT_FOUND;
    }

    esp_err_t err =
        httpd_stop(s_server);

    if (err != ESP_OK)
    {
        return NFW_STATUS_ERROR;
    }

    s_server = NULL;

    ESP_LOGI(
        TAG,
        "OTA HTTP server stopped"
    );

    return NFW_STATUS_OK;
}
