#pragma once

class Calibration {
public:
    explicit Calibration(int dryRaw = 3000, int wetRaw = 1200);

    void setDry(int raw);
    void setWet(int raw);
    int dryRaw() const;
    int wetRaw() const;

    // Converts a raw ADC reading to a 0-100 moisture percentage,
    // linearly interpolated between the wet and dry calibration points
    // and clamped to [0, 100].
    float toPercent(int raw) const;

private:
    int dryRaw_;
    int wetRaw_;
};
