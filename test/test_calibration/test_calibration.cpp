#include <unity.h>
#include "Calibration.h"

void setUp(void) {}
void tearDown(void) {}

void test_default_calibration_midpoint(void) {
    Calibration cal; // defaults: dry=3000, wet=1200
    float pct = cal.toPercent(2100);
    TEST_ASSERT_FLOAT_WITHIN(1.0, 50.0, pct);
}

void test_dry_raw_is_zero_percent(void) {
    Calibration cal(3000, 1200);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, cal.toPercent(3000));
}

void test_wet_raw_is_hundred_percent(void) {
    Calibration cal(3000, 1200);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 100.0, cal.toPercent(1200));
}

void test_clamps_above_dry_raw(void) {
    Calibration cal(3000, 1200);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, cal.toPercent(3500));
}

void test_clamps_below_wet_raw(void) {
    Calibration cal(3000, 1200);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 100.0, cal.toPercent(800));
}

void test_set_dry_and_wet_update_calibration(void) {
    Calibration cal(3000, 1200);
    cal.setDry(2800);
    cal.setWet(1000);
    TEST_ASSERT_EQUAL_INT(2800, cal.dryRaw());
    TEST_ASSERT_EQUAL_INT(1000, cal.wetRaw());
}

void test_dry_equals_wet_returns_zero_without_crashing(void) {
    Calibration cal(2000, 2000);
    TEST_ASSERT_FLOAT_WITHIN(0.01, 0.0, cal.toPercent(2000));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_default_calibration_midpoint);
    RUN_TEST(test_dry_raw_is_zero_percent);
    RUN_TEST(test_wet_raw_is_hundred_percent);
    RUN_TEST(test_clamps_above_dry_raw);
    RUN_TEST(test_clamps_below_wet_raw);
    RUN_TEST(test_set_dry_and_wet_update_calibration);
    RUN_TEST(test_dry_equals_wet_returns_zero_without_crashing);
    return UNITY_END();
}
