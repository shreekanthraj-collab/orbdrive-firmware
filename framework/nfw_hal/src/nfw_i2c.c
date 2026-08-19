/*
 * @file nfw_i2c.c
 * @brief ESP-IDF implementation of the Orb Drive I2C HAL.
 */

#include "nfw_i2c.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"

static bool s_initialized = false;
static NfwI2cConfig_t s_config;

NfwStatus_t nfwI2cInit(const NfwI2cConfig_t *config)
{
    if (config == NULL) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (config->sdaPin > 48U ||
        config->sclPin > 48U ||
        config->frequencyHz == 0U) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    if (s_initialized) {
        return NFW_STATUS_ALREADY_INITIALIZED;
    }

    i2c_config_t i2cConfig = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = (gpio_num_t)config->sdaPin,
        .scl_io_num = (gpio_num_t)config->sclPin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = config->frequencyHz
    };

    esp_err_t err = i2c_param_config(
        (i2c_port_t)config->port,
        &i2cConfig);

    if (err != ESP_OK) {
        return NFW_STATUS_ERROR;
    }

    err = i2c_driver_install(
        (i2c_port_t)config->port,
        I2C_MODE_MASTER,
        0,
        0,
        0);

    if (err != ESP_OK) {
        return NFW_STATUS_ERROR;
    }

    s_config = *config;
    s_initialized = true;

    return NFW_STATUS_OK;
}

NfwStatus_t nfwI2cWrite(
    uint8_t address,
    const uint8_t *data,
    uint32_t length)
{
    if (!s_initialized ||
        data == NULL ||
        length == 0U) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if (cmd == NULL) {
        return NFW_STATUS_NO_MEMORY;
    }

    esp_err_t err = i2c_master_start(cmd);

    if (err == ESP_OK) {
        err = i2c_master_write_byte(
            cmd,
            (uint8_t)((address << 1U) | I2C_MASTER_WRITE),
            true);
    }

    if (err == ESP_OK) {
        err = i2c_master_write(
            cmd,
            (uint8_t *)data,
            length,
            true);
    }

    if (err == ESP_OK) {
        err = i2c_master_stop(cmd);
    }

    if (err == ESP_OK) {
        err = i2c_master_cmd_begin(
            (i2c_port_t)s_config.port,
            cmd,
            pdMS_TO_TICKS(100U));
    }

    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK) {
        return NFW_STATUS_OK;
    }

    if (err == ESP_ERR_TIMEOUT) {
        return NFW_STATUS_TIMEOUT;
    }

    return NFW_STATUS_ERROR;
}

NfwStatus_t nfwI2cRead(
    uint8_t address,
    uint8_t *data,
    uint32_t length)
{
    if (!s_initialized ||
        data == NULL ||
        length == 0U) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if (cmd == NULL) {
        return NFW_STATUS_NO_MEMORY;
    }

    esp_err_t err = i2c_master_start(cmd);

    if (err == ESP_OK) {
        err = i2c_master_write_byte(
            cmd,
            (uint8_t)((address << 1U) | I2C_MASTER_READ),
            true);
    }

    if (err == ESP_OK) {
        err = i2c_master_read(
            cmd,
            data,
            length,
            I2C_MASTER_LAST_NACK);
    }

    if (err == ESP_OK) {
        err = i2c_master_stop(cmd);
    }

    if (err == ESP_OK) {
        err = i2c_master_cmd_begin(
            (i2c_port_t)s_config.port,
            cmd,
            pdMS_TO_TICKS(100U));
    }

    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK) {
        return NFW_STATUS_OK;
    }

    if (err == ESP_ERR_TIMEOUT) {
        return NFW_STATUS_TIMEOUT;
    }

    return NFW_STATUS_ERROR;
}

NfwStatus_t nfwI2cWriteRead(
    uint8_t address,
    const uint8_t *writeData,
    uint32_t writeLength,
    uint8_t *readData,
    uint32_t readLength)
{
    if (!s_initialized ||
        writeData == NULL ||
        readData == NULL ||
        writeLength == 0U ||
        readLength == 0U) {
        return NFW_STATUS_INVALID_ARGUMENT;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if (cmd == NULL) {
        return NFW_STATUS_NO_MEMORY;
    }

    esp_err_t err = i2c_master_start(cmd);

    if (err == ESP_OK) {
        err = i2c_master_write_byte(
            cmd,
            (uint8_t)((address << 1U) | I2C_MASTER_WRITE),
            true);
    }

    if (err == ESP_OK) {
        err = i2c_master_write(
            cmd,
            (uint8_t *)writeData,
            writeLength,
            true);
    }

    if (err == ESP_OK) {
        err = i2c_master_start(cmd);
    }

    if (err == ESP_OK) {
        err = i2c_master_write_byte(
            cmd,
            (uint8_t)((address << 1U) | I2C_MASTER_READ),
            true);
    }

    if (err == ESP_OK) {
        err = i2c_master_read(
            cmd,
            readData,
            readLength,
            I2C_MASTER_LAST_NACK);
    }

    if (err == ESP_OK) {
        err = i2c_master_stop(cmd);
    }

    if (err == ESP_OK) {
        err = i2c_master_cmd_begin(
            (i2c_port_t)s_config.port,
            cmd,
            pdMS_TO_TICKS(100U));
    }

    i2c_cmd_link_delete(cmd);

    if (err == ESP_OK) {
        return NFW_STATUS_OK;
    }

    if (err == ESP_ERR_TIMEOUT) {
        return NFW_STATUS_TIMEOUT;
    }

    return NFW_STATUS_ERROR;
}

bool nfwI2cDevicePresent(uint8_t address)
{
    if (!s_initialized) {
        return false;
    }

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    if (cmd == NULL) {
        return false;
    }

    esp_err_t err = i2c_master_start(cmd);

    if (err == ESP_OK) {
        err = i2c_master_write_byte(
            cmd,
            (uint8_t)((address << 1U) | I2C_MASTER_WRITE),
            true);
    }

    if (err == ESP_OK) {
        err = i2c_master_stop(cmd);
    }

    if (err == ESP_OK) {
        err = i2c_master_cmd_begin(
            (i2c_port_t)s_config.port,
            cmd,
            pdMS_TO_TICKS(100U));
    }

    i2c_cmd_link_delete(cmd);

    return (err == ESP_OK);
}
