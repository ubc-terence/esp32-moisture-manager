#include "ReadingStability.h"
#include <algorithm>

ReadingStability::ReadingStability(size_t windowSize, int toleranceRaw)
    : windowSize_(windowSize), toleranceRaw_(toleranceRaw) {}

void ReadingStability::addReading(int raw) {
    if (window_.size() >= windowSize_) {
        window_.erase(window_.begin());
    }
    window_.push_back(raw);
}

bool ReadingStability::isStable() const {
    if (window_.size() < windowSize_) {
        return false;
    }
    int lo = *std::min_element(window_.begin(), window_.end());
    int hi = *std::max_element(window_.begin(), window_.end());
    return (hi - lo) <= toleranceRaw_;
}
