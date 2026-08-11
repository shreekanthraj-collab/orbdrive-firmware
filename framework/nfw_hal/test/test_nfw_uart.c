#include "unity.h"
#include "nfw_uart.h"

void setUp(void)
{
}

void tearDown(void)
{
}

TEST_CASE("UART init rejects NULL configuration", "[nfw_uart]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwUartInit(NULL));
}

TEST_CASE("UART init rejects invalid data bits", "[nfw_uart]")
{
    NfwUartConfig_t config = {
        .port = 0U,
        .baudRate = 115200U,
        .dataBits = 4U,
        .stopBits = 1U,
        .parityEnabled = false,
.txPin = 15,
.rxPin = 16
    };

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwUartInit(&config));
}

TEST_CASE("UART init rejects invalid stop bits", "[nfw_uart]")
{
    NfwUartConfig_t config = {
        .port = 0U,
        .baudRate = 115200U,
        .dataBits = 8U,
        .stopBits = 3U,
        .parityEnabled = false,
.txPin = 15,
.rxPin = 16
    };

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwUartInit(&config));
}

TEST_CASE("UART write rejects NULL data", "[nfw_uart]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwUartWrite(0U, NULL, 1U));
}

TEST_CASE("UART write rejects zero length", "[nfw_uart]")
{
    uint8_t data = 0U;

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwUartWrite(0U, &data, 0U));
}

TEST_CASE("UART read rejects NULL data", "[nfw_uart]")
{
    uint8_t buffer[4];
    uint32_t bytesRead = 0U;

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwUartRead(0U, NULL, sizeof(buffer), &bytesRead));
}

TEST_CASE("UART read rejects NULL bytesRead", "[nfw_uart]")
{
    uint8_t buffer[4];

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwUartRead(0U, buffer, sizeof(buffer), NULL));
}

TEST_CASE("UART read rejects zero length", "[nfw_uart]")
{
    uint8_t buffer[4];
    uint32_t bytesRead = 0U;

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwUartRead(0U, buffer, 0U, &bytesRead));
}