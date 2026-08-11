#include "unity.h"
#include "fault_manager.h"

static FaultManagerContext_t context;

void setUp(void)
{
	faultManagerInit(&context);
}

void tearDown(void)
{
}

TEST_CASE("Fault manager initializes to no fault", "[fault_manager]")
{
	TEST_ASSERT_EQUAL(
		FAULT_CODE_NONE,
		faultManagerGetActive(&context));

	TEST_ASSERT_FALSE(context.fault_latched);
}

TEST_CASE("Voltage fault is reported as recoverable", "[fault_manager]")
{
	faultManagerReport(&context, FAULT_CODE_VOLTAGE);

	TEST_ASSERT_EQUAL(
		FAULT_CODE_VOLTAGE,
		faultManagerGetActive(&context));

	TEST_ASSERT_FALSE(context.fault_latched);
}

TEST_CASE("Overcurrent replaces lower priority voltage fault", "[fault_manager]")
{
	faultManagerReport(&context, FAULT_CODE_VOLTAGE);
	faultManagerReport(&context, FAULT_CODE_OVERCURRENT);

	TEST_ASSERT_EQUAL(
		FAULT_CODE_OVERCURRENT,
		faultManagerGetActive(&context));

	TEST_ASSERT_TRUE(context.fault_latched);
}

TEST_CASE("Lower priority I2C fault does not replace overcurrent", "[fault_manager]")
{
	faultManagerReport(&context, FAULT_CODE_OVERCURRENT);
	faultManagerReport(&context, FAULT_CODE_I2C);

	TEST_ASSERT_EQUAL(
		FAULT_CODE_OVERCURRENT,
		faultManagerGetActive(&context));

	TEST_ASSERT_TRUE(context.fault_latched);
}

TEST_CASE("Lock replaces overcurrent", "[fault_manager]")
{
	faultManagerReport(&context, FAULT_CODE_OVERCURRENT);
	faultManagerReport(&context, FAULT_CODE_LOCK);

	TEST_ASSERT_EQUAL(
		FAULT_CODE_LOCK,
		faultManagerGetActive(&context));

	TEST_ASSERT_TRUE(context.fault_latched);
}

TEST_CASE("Wrong fault cannot be cleared", "[fault_manager]")
{
	faultManagerReport(&context, FAULT_CODE_OVERCURRENT);

	faultManagerClear(&context, FAULT_CODE_VOLTAGE);

	TEST_ASSERT_EQUAL(
		FAULT_CODE_OVERCURRENT,
		faultManagerGetActive(&context));

	TEST_ASSERT_TRUE(context.fault_latched);
}

TEST_CASE("Active fault can be explicitly cleared", "[fault_manager]")
{
	faultManagerReport(&context, FAULT_CODE_OVERCURRENT);

	faultManagerClear(&context, FAULT_CODE_OVERCURRENT);

	TEST_ASSERT_EQUAL(
		FAULT_CODE_NONE,
		faultManagerGetActive(&context));

	TEST_ASSERT_FALSE(context.fault_latched);
}

TEST_CASE("Reporting NONE does not clear active fault", "[fault_manager]")
{
	faultManagerReport(&context, FAULT_CODE_OVERCURRENT);
	faultManagerReport(&context, FAULT_CODE_NONE);

	TEST_ASSERT_EQUAL(
		FAULT_CODE_OVERCURRENT,
		faultManagerGetActive(&context));

	TEST_ASSERT_TRUE(context.fault_latched);
}
TEST_CASE("Clearing NONE does not clear active fault", "[fault_manager]")
{
    faultManagerReport(&context, FAULT_CODE_OVERCURRENT);
    faultManagerClear(&context, FAULT_CODE_NONE);

    TEST_ASSERT_EQUAL(
        FAULT_CODE_OVERCURRENT,
        faultManagerGetActive(&context));

    TEST_ASSERT_TRUE(context.fault_latched);
}
TEST_CASE("NULL context is handled safely", "[fault_manager]")
{
	faultManagerInit(NULL);
	faultManagerReport(NULL, FAULT_CODE_OVERCURRENT);
	faultManagerClear(NULL, FAULT_CODE_OVERCURRENT);

	TEST_ASSERT_EQUAL(
		FAULT_CODE_NONE,
		faultManagerGetActive(NULL));
}
