#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include <ArduinoJson.h>

struct HistorySample {
    uint32_t index;
    float percent;
};

class HistoryBuffer {
public:
    explicit HistoryBuffer(size_t capacity);

    // Adds a new sample, auto-assigning the next sample index. If the
    // buffer is already at capacity, the oldest sample is dropped.
    void addSample(float percent);

    size_t size() const;
    size_t capacity() const;

    // Oldest-to-newest access, i in [0, size()).
    HistorySample sampleAt(size_t i) const;

    // Index of the most recently added sample (0 if none added yet).
    uint32_t lastIndex() const;
    bool hasSamples() const;

    // Fills a JSON array with {age_seconds, moisture_pct} oldest-to-newest.
    // If the buffer holds more than maxPoints records, an evenly-strided
    // subset of at most maxPoints records is emitted instead (always
    // including the most recent record) to bound the JSON payload size.
    void toHistoryJson(JsonArray &arr, uint16_t intervalSeconds, size_t maxPoints = SIZE_MAX) const;

    // Restores persisted state (used by HistoryStore on boot). Trims to
    // capacity() if given more records than fit, keeping the newest.
    void restoreFrom(const std::vector<HistorySample> &records);

    // Read-only access to current records, oldest-to-newest (for persistence).
    const std::vector<HistorySample>& records() const;

private:
    size_t capacity_;
    std::vector<HistorySample> records_; // oldest at front, newest at back
    uint32_t nextIndex_ = 0;
};
