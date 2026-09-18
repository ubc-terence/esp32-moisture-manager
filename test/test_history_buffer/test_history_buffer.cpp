#include <unity.h>
#include "HistoryBuffer.h"

void setUp(void) {}
void tearDown(void) {}

void test_empty_buffer_has_zero_size(void) {
    HistoryBuffer buf(3);
    TEST_ASSERT_EQUAL(0, buf.size());
    TEST_ASSERT_FALSE(buf.hasSamples());
}

void test_add_sample_increases_size(void) {
    HistoryBuffer buf(3);
    buf.addSample(42.0f);
    TEST_ASSERT_EQUAL(1, buf.size());
    TEST_ASSERT_TRUE(buf.hasSamples());
    TEST_ASSERT_EQUAL_FLOAT(42.0f, buf.sampleAt(0).percent);
}

void test_wraps_after_capacity_exceeded(void) {
    HistoryBuffer buf(3);
    buf.addSample(10.0f); // index 0
    buf.addSample(20.0f); // index 1
    buf.addSample(30.0f); // index 2
    buf.addSample(40.0f); // index 3, drops index 0

    TEST_ASSERT_EQUAL(3, buf.size());
    TEST_ASSERT_EQUAL_UINT32(1, buf.sampleAt(0).index);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, buf.sampleAt(0).percent);
    TEST_ASSERT_EQUAL_UINT32(3, buf.sampleAt(2).index);
    TEST_ASSERT_EQUAL_FLOAT(40.0f, buf.sampleAt(2).percent);
}

void test_to_history_json_age_seconds(void) {
    HistoryBuffer buf(3);
    buf.addSample(10.0f); // index 0
    buf.addSample(20.0f); // index 1
    buf.addSample(30.0f); // index 2 (most recent)

    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    buf.toHistoryJson(arr, 600);

    TEST_ASSERT_EQUAL(3, arr.size());
    TEST_ASSERT_EQUAL_UINT32(1200, arr[0]["age_seconds"].as<uint32_t>());
    TEST_ASSERT_EQUAL_UINT32(600, arr[1]["age_seconds"].as<uint32_t>());
    TEST_ASSERT_EQUAL_UINT32(0, arr[2]["age_seconds"].as<uint32_t>());
    TEST_ASSERT_EQUAL_FLOAT(30.0f, arr[2]["moisture_pct"].as<float>());
}

void test_restore_from_continues_index_sequence(void) {
    HistoryBuffer buf(5);
    std::vector<HistorySample> persisted = {
        {3, 15.0f},
        {4, 16.0f},
        {5, 17.0f},
    };
    buf.restoreFrom(persisted);

    TEST_ASSERT_EQUAL(3, buf.size());
    TEST_ASSERT_EQUAL_UINT32(5, buf.lastIndex());

    buf.addSample(18.0f);
    TEST_ASSERT_EQUAL_UINT32(6, buf.sampleAt(3).index);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_empty_buffer_has_zero_size);
    RUN_TEST(test_add_sample_increases_size);
    RUN_TEST(test_wraps_after_capacity_exceeded);
    RUN_TEST(test_to_history_json_age_seconds);
    RUN_TEST(test_restore_from_continues_index_sequence);
    return UNITY_END();
}
