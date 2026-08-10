#include "unity.h"
#include "nfw_lifecycle.h"

void setUp(void)
{
}

void tearDown(void)
{
}

TEST_CASE("Lifecycle initializes to RESET", "[nfw_lifecycle]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleInit());

    TEST_ASSERT_EQUAL(
        NFW_LIFECYCLE_STAGE_RESET,
        nfwLifecycleGetStage());

    TEST_ASSERT_FALSE(nfwLifecycleIsRunning());
}

TEST_CASE("Repeated lifecycle initialization is rejected", "[nfw_lifecycle]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleInit());

    TEST_ASSERT_EQUAL(
        NFW_STATUS_ALREADY_INITIALIZED,
        nfwLifecycleInit());
}

TEST_CASE("Lifecycle cannot advance before initialization", "[nfw_lifecycle]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_STATE,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_HAL_INIT));
}

TEST_CASE("Lifecycle advances through all valid stages", "[nfw_lifecycle]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleInit());

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_HAL_INIT));

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_PLATFORM_INIT));

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_SERVICES_INIT));

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_APPLICATION_INIT));

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_RUNNING));

    TEST_ASSERT_EQUAL(
        NFW_LIFECYCLE_STAGE_RUNNING,
        nfwLifecycleGetStage());

    TEST_ASSERT_TRUE(nfwLifecycleIsRunning());
}

TEST_CASE("Lifecycle rejects skipped stage", "[nfw_lifecycle]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleInit());

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_STATE,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_PLATFORM_INIT));
}

TEST_CASE("Lifecycle rejects backward transition", "[nfw_lifecycle]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleInit());

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_HAL_INIT));

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_STATE,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_RESET));
}

TEST_CASE("RUNNING is terminal", "[nfw_lifecycle]")
{
    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleInit());

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_HAL_INIT));

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_PLATFORM_INIT));

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_SERVICES_INIT));

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_APPLICATION_INIT));

    TEST_ASSERT_EQUAL(
        NFW_STATUS_OK,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_RUNNING));

    TEST_ASSERT_EQUAL(
        NFW_STATUS_INVALID_STATE,
        nfwLifecycleAdvance(
            NFW_LIFECYCLE_STAGE_RUNNING));

    TEST_ASSERT_TRUE(nfwLifecycleIsRunning());
}
