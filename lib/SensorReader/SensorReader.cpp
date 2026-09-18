#include "SensorReader.h"
#include <Arduino.h>
#include <algorithm>

namespace {
constexpr int SAMPLE_COUNT = 8;
constexpr int POWER_SETTLE_MS = 80;
constexpr int SAMPLE_DELAY_MS = 5;
}

namespace SensorReader {

int readRawAveraged(uint8_t aoutPin, uint8_t vccPin) {
    pinMode(vccPin, OUTPUT);
    digitalWrite(vccPin, HIGH);
    delay(POWER_SETTLE_MS);

    int samples[SAMPLE_COUNT];
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        samples[i] = analogRead(aoutPin);
        delay(SAMPLE_DELAY_MS);
    }

    digitalWrite(vccPin, LOW);

    std::sort(samples, samples + SAMPLE_COUNT);
    long sum = 0;
    for (int i = 1; i < SAMPLE_COUNT - 1; i++) {
        sum += samples[i];
    }
    return static_cast<int>(sum / (SAMPLE_COUNT - 2));
}

}
