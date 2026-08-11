#include "unity.h"
#include "rs485.h"

static uint8_t buffer[8];
static uint32_t bytes_read;

void setUp(void)
{
    bytes_read = 0U;
}

void tearDown(void)
{
}

TEST_CASE("RS-485 init rejects NULL configuration", "[rs485]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        rs485Init(NULL));
}

TEST_CASE("RS-485 transmit rejects NULL data", "[rs485]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        rs485Transmit(
            NULL,
            1U,
            100U));
}

TEST_CASE("RS-485 transmit rejects zero length", "[rs485]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        rs485Transmit(
            buffer,
            0U,
            100U));
}

TEST_CASE("RS-485 receive rejects NULL data", "[rs485]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        rs485Receive(
            NULL,
            sizeof(buffer),
            &bytes_read));
}

TEST_CASE("RS-485 receive rejects NULL bytes read", "[rs485]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        rs485Receive(
            buffer,
            sizeof(buffer),
            NULL));
}

TEST_CASE("RS-485 receive rejects zero length", "[rs485]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        rs485Receive(
            buffer,
            0U,
            &bytes_read));
}

TEST_CASE("RS-485 transmit rejects use before initialization", "[rs485]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_ERROR,
        rs485Transmit(
            buffer,
            sizeof(buffer),
            100U));
}

TEST_CASE("RS-485 receive rejects use before initialization", "[rs485]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_ERROR,
        rs485Receive(
            buffer,
            sizeof(buffer),
            &bytes_read));
}

TEST_CASE("RS-485 deinit is safe before initialization", "[rs485]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        rs485Deinit());
}