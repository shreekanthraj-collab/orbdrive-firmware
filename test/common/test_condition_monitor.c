#include "unity.h"
#include "condition_monitor.h"

static ConditionMonitorContext_t context;
static ConditionMonitorInput_t input;
static ConditionMonitorResult_t result;

void setUp(void)
{
    conditionMonitorInit(&context);
    input.voltage_v = 12.0f;
    input.current_a = 0.0f;
    input.encoder_position_turns = 0.0f;
    input.encoder_movement_turns = 0.0f;
    input.motor_active = false;
    input.current_alert = false;
    input.forced_close_complete = false;
    input.ina226_healthy = true;
    input.as5600_healthy = true;
    input.i2c_healthy = true;

    result.state = CONDITION_STATE_NORMAL;
    result.voltage_state = VOLTAGE_STATE_NORMAL;
    result.fault = FAULT_CODE_NONE;
    result.motor_stop_required = false;
    result.lock_required = false;
    result.force_close_required = false;
}

void tearDown(void)
{
}

TEST_CASE("Condition monitor initializes to normal", "[condition_monitor]")
{
    TEST_ASSERT_EQUAL(VOLTAGE_STATE_NORMAL, context.voltage_state);
    TEST_ASSERT_EQUAL(0, context.overcurrent_retry_count);
    TEST_ASSERT_FALSE(context.overcurrent_fault_active);
    TEST_ASSERT_FALSE(context.force_close_pending);
}

TEST_CASE("Normal voltage produces normal condition", "[condition_monitor]")
{
    conditionMonitorEvaluate(&context, &input, 1000U, &result);
    TEST_ASSERT_EQUAL(CONDITION_STATE_NORMAL, result.state);
    TEST_ASSERT_EQUAL(VOLTAGE_STATE_NORMAL, result.voltage_state);
    TEST_ASSERT_EQUAL(FAULT_CODE_NONE, result.fault);
}

TEST_CASE("Low voltage enters bypass wait", "[condition_monitor]")
{
    input.voltage_v = 11.5f;
    conditionMonitorEvaluate(&context, &input, 1000U, &result);
    TEST_ASSERT_EQUAL(VOLTAGE_STATE_WAIT_BYPASS, result.voltage_state);
    TEST_ASSERT_EQUAL(CONDITION_STATE_FAULT, result.state);
    TEST_ASSERT_EQUAL(FAULT_CODE_VOLTAGE, result.fault);
    TEST_ASSERT_TRUE(result.motor_stop_required);
}

TEST_CASE("Bypass wait remains active before timeout", "[condition_monitor]")
{
    input.voltage_v = 11.5f;
    conditionMonitorEvaluate(&context, &input, 1000U, &result);
    conditionMonitorEvaluate(&context, &input, 120999U, &result);
    TEST_ASSERT_EQUAL(VOLTAGE_STATE_WAIT_BYPASS, result.voltage_state);
}

TEST_CASE("Bypass wait locks at timeout", "[condition_monitor]")
{
    input.voltage_v = 11.5f;
    conditionMonitorEvaluate(&context, &input, 1000U, &result);
    conditionMonitorEvaluate(&context, &input, 121000U, &result);
    TEST_ASSERT_EQUAL(VOLTAGE_STATE_LOCKED, result.voltage_state);
    TEST_ASSERT_EQUAL(CONDITION_STATE_LOCKED, result.state);
    TEST_ASSERT_EQUAL(FAULT_CODE_LOCK, result.fault);
    TEST_ASSERT_TRUE(result.motor_stop_required);
    TEST_ASSERT_TRUE(result.lock_required);
}

TEST_CASE("Voltage recovery exits bypass wait", "[condition_monitor]")
{
    input.voltage_v = 11.5f;
    conditionMonitorEvaluate(&context, &input, 1000U, &result);
    input.voltage_v = 12.0f;
    conditionMonitorEvaluate(&context, &input, 2000U, &result);
    TEST_ASSERT_EQUAL(VOLTAGE_STATE_NORMAL, result.voltage_state);
    TEST_ASSERT_EQUAL(CONDITION_STATE_NORMAL, result.state);
    TEST_ASSERT_EQUAL(FAULT_CODE_NONE, result.fault);
}

TEST_CASE("Critical voltage locks immediately", "[condition_monitor]")
{
    input.voltage_v = 11.1f;
    conditionMonitorEvaluate(&context, &input, 1000U, &result);
    TEST_ASSERT_EQUAL(VOLTAGE_STATE_LOCKED, result.voltage_state);
    TEST_ASSERT_EQUAL(CONDITION_STATE_LOCKED, result.state);
    TEST_ASSERT_EQUAL(FAULT_CODE_LOCK, result.fault);
    TEST_ASSERT_TRUE(result.motor_stop_required);
    TEST_ASSERT_TRUE(result.lock_required);
}

TEST_CASE("Overcurrent trip starts retry interval", "[condition_monitor]")
{
    input.current_a = 2.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        1000U,
        &result);

    TEST_ASSERT_EQUAL(
        1,
        context.overcurrent_retry_count);

    TEST_ASSERT_TRUE(context.last_overcurrent);
    TEST_ASSERT_FALSE(context.overcurrent_fault_active);

    TEST_ASSERT_EQUAL(
        FAULT_CODE_OVERCURRENT,
        result.fault);

    TEST_ASSERT_TRUE(result.motor_stop_required);
}

TEST_CASE("Overcurrent retry interval clears after delay", "[condition_monitor]")
{
    input.current_a = 2.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        1000U,
        &result);

    input.current_a = 0.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        31000U,
        &result);

    TEST_ASSERT_FALSE(context.last_overcurrent);
    TEST_ASSERT_FALSE(context.overcurrent_fault_active);

    TEST_ASSERT_EQUAL(
        1,
        context.overcurrent_retry_count);
}

TEST_CASE("Repeated overcurrent trips eventually fault", "[condition_monitor]")
{
    input.current_a = 2.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        1000U,
        &result);

    input.current_a = 0.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        31000U,
        &result);

    input.current_a = 2.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        32000U,
        &result);

    input.current_a = 0.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        62000U,
        &result);

    input.current_a = 2.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        63000U,
        &result);

    TEST_ASSERT_TRUE(context.overcurrent_fault_active);
    TEST_ASSERT_TRUE(context.force_close_pending);

    TEST_ASSERT_EQUAL(
        FAULT_CODE_OVERCURRENT,
        result.fault);

    TEST_ASSERT_TRUE(result.motor_stop_required);
    TEST_ASSERT_TRUE(result.force_close_required);
}

TEST_CASE("Completed forced close locks after overcurrent fault", "[condition_monitor]")
{
    input.current_a = 2.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        1000U,
        &result);

    input.current_a = 0.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        31000U,
        &result);

    input.current_a = 2.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        32000U,
        &result);

    input.current_a = 0.0f;

    conditionMonitorEvaluate(
        &context,
        &input,
        62000U,
        &result);

    input.current_a = 2.0f;
    input.forced_close_complete = false;

    conditionMonitorEvaluate(
        &context,
        &input,
        63000U,
        &result);

    input.forced_close_complete = true;

    conditionMonitorEvaluate(
        &context,
        &input,
        64000U,
        &result);

    TEST_ASSERT_EQUAL(
        CONDITION_STATE_LOCKED,
        result.state);

    TEST_ASSERT_EQUAL(
        FAULT_CODE_OVERCURRENT,
        result.fault);

    TEST_ASSERT_TRUE(result.motor_stop_required);
    TEST_ASSERT_TRUE(result.lock_required);
}

TEST_CASE("NULL input is handled safely", "[condition_monitor]")
{
    conditionMonitorEvaluate(&context, NULL, 1000U, &result);
    TEST_ASSERT_EQUAL(CONDITION_STATE_NORMAL, result.state);
}
