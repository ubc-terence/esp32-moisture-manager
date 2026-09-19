#include <unity.h>
#include "ReadingStability.h"

void setUp(void) {}
void tearDown(void) {}

void test_not_stable_before_window_full(void) {
    ReadingStability rs(3, 15);
    rs.addReading(100);
    rs.addReading(102);
    TEST_ASSERT_FALSE(rs.isStable());
}

void test_stable_when_readings_within_tolerance(void) {
    ReadingStability rs(3, 15);
    rs.addReading(100);
    rs.addReading(105);
    rs.addReading(102);
    TEST_ASSERT_TRUE(rs.isStable());
}

void test_not_stable_when_readings_spread_wide(void) {
    ReadingStability rs(3, 15);
    rs.addReading(3000);
    rs.addReading(1500);
    rs.addReading(300);
    TEST_ASSERT_FALSE(rs.isStable());
}

void test_window_slides_and_recovers_stability(void) {
    ReadingStability rs(3, 15);
    rs.addReading(3000); // drifting
    rs.addReading(1500);
    rs.addReading(300);
    TEST_ASSERT_FALSE(rs.isStable());

    rs.addReading(295); // window is now {1500, 300, 295} - still unstable
    TEST_ASSERT_FALSE(rs.isStable());

    rs.addReading(292); // window is now {300, 295, 292}, spread 8 - stable
    TEST_ASSERT_TRUE(rs.isStable());
}

void test_boundary_spread_equal_to_tolerance_is_stable(void) {
    ReadingStability rs(3, 15);
    rs.addReading(100);
    rs.addReading(115); // spread of exactly 15
    rs.addReading(105);
    TEST_ASSERT_TRUE(rs.isStable());
}

void test_custom_window_and_tolerance(void) {
    ReadingStability rs(2, 5);
    rs.addReading(500);
    rs.addReading(504); // spread 4, within tolerance 5, window of 2 is full
    TEST_ASSERT_TRUE(rs.isStable());

    rs.addReading(520); // window is now {504, 520}, spread 16, unstable
    TEST_ASSERT_FALSE(rs.isStable());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_not_stable_before_window_full);
    RUN_TEST(test_stable_when_readings_within_tolerance);
    RUN_TEST(test_not_stable_when_readings_spread_wide);
    RUN_TEST(test_window_slides_and_recovers_stability);
    RUN_TEST(test_boundary_spread_equal_to_tolerance_is_stable);
    RUN_TEST(test_custom_window_and_tolerance);
    return UNITY_END();
}
