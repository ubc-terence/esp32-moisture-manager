#pragma once
#include <cstddef>
#include <vector>

// Tracks a sliding window of recent raw sensor readings and reports whether
// they've stopped drifting, so callers can avoid trusting (or calibrating
// from) a reading that's still settling after a fresh power-on/reset.
class ReadingStability {
public:
    explicit ReadingStability(size_t windowSize = 3, int toleranceRaw = 15);

    // Records a new raw reading, evicting the oldest once the window is full.
    void addReading(int raw);

    // True once at least windowSize readings have been recorded and the
    // spread (max - min) across the current window is within toleranceRaw.
    bool isStable() const;

private:
    size_t windowSize_;
    int toleranceRaw_;
    std::vector<int> window_; // oldest at front, newest at back
};
