#include "unity.h"
#include "nfw_gpio.h"

void setUp(void)
{
}

void tearDown(void)
{
}

TEST_CASE("GPIO init rejects NULL configuration", "[nfw_gpio]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwGpioInit(NULL));
}

TEST_CASE("GPIO init rejects invalid pin", "[nfw_gpio]")
{
    NfwGpioConfig_t config = {
        .pin = UINT32_MAX,
        .direction = NFW_GPIO_DIRECTION_INPUT,
        .pull = NFW_GPIO_PULL_NONE,
        .initialLevel = false
    };

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwGpioInit(&config));
}

TEST_CASE("GPIO read rejects NULL level pointer", "[nfw_gpio]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwGpioRead(0U, NULL));
}

TEST_CASE("GPIO read rejects invalid pin", "[nfw_gpio]")
{
    bool level = false;

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwGpioRead(UINT32_MAX, &level));
}

TEST_CASE("GPIO write rejects invalid pin", "[nfw_gpio]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwGpioWrite(UINT32_MAX, false));
}

TEST_CASE("GPIO toggle rejects invalid pin", "[nfw_gpio]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_ARGUMENT,
        nfwGpioToggle(UINT32_MAX));
}