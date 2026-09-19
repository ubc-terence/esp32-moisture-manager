#pragma once
#include <ESPAsyncWebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "HistoryBuffer.h"
#include "Calibration.h"

namespace WebServer {

struct Context {
    HistoryBuffer *history;
    Calibration *calibration;
    uint16_t intervalSeconds;
    int lastRawReading;
    float lastPercent;
    bool stable;
    // Guards access to *history and *calibration, which are shared between
    // the Arduino loop() task and the AsyncTCP task that runs these route
    // handlers.
    SemaphoreHandle_t mutex;
};

// Registers all routes on server. ctx must outlive server.
void setup(AsyncWebServer &server, Context &ctx);

}
