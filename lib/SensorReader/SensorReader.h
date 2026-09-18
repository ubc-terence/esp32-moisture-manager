#pragma once
#include <cstdint>

namespace SensorReader {

// Powers the sensor via vccPin, averages several ADC reads on aoutPin
// (discarding the min and max outlier), then powers it back down.
// Returns the averaged raw ADC value (0-4095 on the ESP32-C3's 12-bit ADC).
int readRawAveraged(uint8_t aoutPin, uint8_t vccPin);

}
