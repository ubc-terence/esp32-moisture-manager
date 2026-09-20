// PSRAM check: reports whether PSRAM is actually usable in this build.
//
// On the Waveshare ESP32-C5-WIFI6-KIT with the pinned pioarduino platform the
// boot log prints "E MSPI Timing: Failed to allocate dummy cacheline for PSRAM
// memory barrier!" and PSRAM does not initialize (psramFound() == 0). The
// firmware only uses internal RAM, so this is harmless today; use this sketch
// to re-test after changing the platform version or board definition.
//
// Build/flash: pio run -e diag-psram-check -t upload --upload-port <port>
// Read output: python tools/capture_serial.py --seconds 15 --reset
// Afterwards re-flash the real firmware: pio run -t upload --upload-port <port>

#include <Arduino.h>
#include "esp_heap_caps.h"

void setup() {
    Serial.begin(115200);
    delay(2500);
    Serial.println("PSRAM_CHECK start");
    Serial.printf("psramFound=%d  getPsramSize=%u  freePsram=%u\n", psramFound(), (unsigned)ESP.getPsramSize(),
                  (unsigned)ESP.getFreePsram());
    Serial.printf("internal heap: total=%u free=%u\n", (unsigned)heap_caps_get_total_size(MALLOC_CAP_INTERNAL),
                  (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL));

    const size_t n = 1024 * 1024;
    uint8_t *p = (uint8_t *)heap_caps_malloc(n, MALLOC_CAP_SPIRAM);
    if (!p) {
        Serial.println("RESULT: PSRAM NOT USABLE (1 MB PSRAM allocation failed)");
    } else {
        for (size_t i = 0; i < n; i++) p[i] = (uint8_t)((i * 31) ^ (i >> 8));
        size_t bad = 0;
        for (size_t i = 0; i < n; i++) {
            if (p[i] != (uint8_t)((i * 31) ^ (i >> 8))) bad++;
        }
        Serial.printf("RESULT: PSRAM USABLE. 1 MB write/read-back mismatches: %u\n", (unsigned)bad);
        free(p);
    }
    Serial.println("PSRAM_CHECK done");
}

void loop() {
    delay(1000);
}
