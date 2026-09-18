#include "Calibration.h"

Calibration::Calibration(int dryRaw, int wetRaw)
    : dryRaw_(dryRaw), wetRaw_(wetRaw) {}

void Calibration::setDry(int raw) { dryRaw_ = raw; }
void Calibration::setWet(int raw) { wetRaw_ = raw; }
int Calibration::dryRaw() const { return dryRaw_; }
int Calibration::wetRaw() const { return wetRaw_; }

float Calibration::toPercent(int raw) const {
    if (dryRaw_ == wetRaw_) {
        return 0.0f;
    }

    float pct = static_cast<float>(dryRaw_ - raw) /
                static_cast<float>(dryRaw_ - wetRaw_) * 100.0f;

    if (pct < 0.0f) pct = 0.0f;
    if (pct > 100.0f) pct = 100.0f;
    return pct;
}
