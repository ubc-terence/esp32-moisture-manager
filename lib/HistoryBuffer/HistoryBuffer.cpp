#include "HistoryBuffer.h"

HistoryBuffer::HistoryBuffer(size_t capacity) : capacity_(capacity) {}

void HistoryBuffer::addSample(float percent) {
    if (records_.size() >= capacity_) {
        records_.erase(records_.begin());
    }
    records_.push_back({nextIndex_, percent});
    nextIndex_++;
}

size_t HistoryBuffer::size() const { return records_.size(); }
size_t HistoryBuffer::capacity() const { return capacity_; }

HistorySample HistoryBuffer::sampleAt(size_t i) const {
    return records_[i];
}

uint32_t HistoryBuffer::lastIndex() const {
    return records_.empty() ? 0 : records_.back().index;
}

bool HistoryBuffer::hasSamples() const {
    return !records_.empty();
}

void HistoryBuffer::toHistoryJson(JsonArray &arr, uint16_t intervalSeconds, size_t maxPoints) const {
    uint32_t last = lastIndex();
    size_t n = records_.size();

    if (n <= maxPoints) {
        for (const auto &record : records_) {
            JsonObject obj = arr.add<JsonObject>();
            obj["age_seconds"] = (last - record.index) * intervalSeconds;
            obj["moisture_pct"] = record.percent;
        }
        return;
    }

    // More records than maxPoints allows: emit an evenly-strided subset,
    // and always include the most recent record so the chart's right edge
    // (age 0) is never missing.
    size_t stride = n / maxPoints;
    if (stride < 1) {
        stride = 1;
    }

    for (size_t i = 0; i < n; i += stride) {
        const auto &record = records_[i];
        JsonObject obj = arr.add<JsonObject>();
        obj["age_seconds"] = (last - record.index) * intervalSeconds;
        obj["moisture_pct"] = record.percent;
    }

    if ((n - 1) % stride != 0) {
        const auto &record = records_[n - 1];
        JsonObject obj = arr.add<JsonObject>();
        obj["age_seconds"] = (last - record.index) * intervalSeconds;
        obj["moisture_pct"] = record.percent;
    }
}

void HistoryBuffer::restoreFrom(const std::vector<HistorySample> &records) {
    records_ = records;
    if (records_.size() > capacity_) {
        records_.erase(records_.begin(), records_.begin() + (records_.size() - capacity_));
    }
    nextIndex_ = records_.empty() ? 0 : records_.back().index + 1;
}

const std::vector<HistorySample>& HistoryBuffer::records() const {
    return records_;
}
