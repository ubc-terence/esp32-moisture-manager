#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "Calibration.h"
#include "CalibrationStore.h"
#include "HistoryBuffer.h"
#include "HistoryStore.h"
#include "SensorReader.h"
#include "WebServer.h"

namespace {
// ESP32-C5-WIFI6-KIT (Waveshare), board = esp32-c5-devkitc-1 pinout.
// GPIO4 (ADC1_CH3) and GPIO8 are both on this board's "safe" pin list:
// no strapping-pin conflicts, not reserved for flash/PSRAM/USB-JTAG/UART0.
constexpr uint8_t AOUT_PIN = 4;
constexpr uint8_t VCC_PIN = 8;
constexpr unsigned long READ_INTERVAL_MS = 10000;       // raw sensor read cadence
constexpr unsigned long PERSIST_INTERVAL_MS = 600000;   // 10 minutes
constexpr size_t HISTORY_CAPACITY = 8640;                // 60 days at 10-minute interval
constexpr uint16_t HISTORY_INTERVAL_SECONDS = PERSIST_INTERVAL_MS / 1000;
constexpr const char *AP_SSID = "PlantMonitor";
constexpr const char *AP_PASSWORD = "plant1234"; // WPA2, >= 8 chars required by ESP-IDF

Calibration calibration;
HistoryBuffer history(HISTORY_CAPACITY);
AsyncWebServer server(80);
WebServer::Context webCtx;

// Guards access to `history` and `calibration`, which are touched by both
// this file's loop() (Arduino task) and WebServer's route handlers (run on
// the separate AsyncTCP task).
SemaphoreHandle_t stateMutex = nullptr;

unsigned long lastReadMs = 0;
unsigned long lastPersistMs = 0;
}

void setup() {
    Serial.begin(115200);

    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS mount failed");
    }
    if (!LittleFS.exists("/index.html")) {
        Serial.println("WARNING: /index.html not found on LittleFS — did you run 'pio run -t uploadfs'?");
    }

    stateMutex = xSemaphoreCreateMutex();

    CalibrationStore::load(calibration);
    HistoryStore::load(history);

    // Seed a real reading before the web server starts so an early
    // dashboard visit (before loop()'s first 10s tick) can't persist a
    // calibration point off of a stale/zero reading.
    int initialRaw = SensorReader::readRawAveraged(AOUT_PIN, VCC_PIN);
    webCtx.lastRawReading = initialRaw;
    webCtx.lastPercent = calibration.toPercent(initialRaw);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    WiFi.setTxPower(WIFI_POWER_19_5dBm); // max, for consistency with the C3 build
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    webCtx.history = &history;
    webCtx.calibration = &calibration;
    webCtx.intervalSeconds = HISTORY_INTERVAL_SECONDS;
    webCtx.mutex = stateMutex;

    WebServer::setup(server, webCtx);
    server.begin();
}

void loop() {
    unsigned long now = millis();

    if (now - lastReadMs >= READ_INTERVAL_MS) {
        lastReadMs = now;
        int raw = SensorReader::readRawAveraged(AOUT_PIN, VCC_PIN);

        xSemaphoreTake(stateMutex, portMAX_DELAY);
        webCtx.lastRawReading = raw;
        webCtx.lastPercent = calibration.toPercent(raw);
        xSemaphoreGive(stateMutex);

        Serial.printf("raw=%d moisture=%.1f%%\n", raw, webCtx.lastPercent);
    }

    if (now - lastPersistMs >= PERSIST_INTERVAL_MS) {
        lastPersistMs = now;

        xSemaphoreTake(stateMutex, portMAX_DELAY);
        history.addSample(webCtx.lastPercent);
        HistoryStore::save(history);
        xSemaphoreGive(stateMutex);

        Serial.println("Persisted history sample");
    }
}
