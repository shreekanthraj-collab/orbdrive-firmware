#include "unity.h"
#include "nfw_delay.h"

void setUp(void)
{
}

void tearDown(void)
{
}

TEST_CASE("Delay milliseconds accepts zero", "[nfw_delay]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwDelayMs(0U));
}

TEST_CASE("Delay microseconds accepts zero", "[nfw_delay]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwDelayUs(0U));
}