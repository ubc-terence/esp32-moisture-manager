#include "WebServer.h"
#include "CalibrationStore.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>

namespace WebServer {

void setup(AsyncWebServer &server, Context &ctx) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html");
    });

    server.on("/api/now", HTTP_GET, [&ctx](AsyncWebServerRequest *request) {
        JsonDocument doc;
        doc["raw"] = ctx.lastRawReading;
        doc["moisture_pct"] = ctx.lastPercent;
        String out;
        serializeJson(doc, out);
        request->send(200, "application/json", out);
    });

    server.on("/api/history", HTTP_GET, [&ctx](AsyncWebServerRequest *request) {
        constexpr size_t MAX_HISTORY_POINTS = 400;
        JsonDocument doc;
        JsonArray arr = doc.to<JsonArray>();

        xSemaphoreTake(ctx.mutex, portMAX_DELAY);
        ctx.history->toHistoryJson(arr, ctx.intervalSeconds, MAX_HISTORY_POINTS);
        xSemaphoreGive(ctx.mutex);

        AsyncResponseStream *response = request->beginResponseStream("application/json");
        serializeJson(doc, *response);
        request->send(response);
    });

    auto *calibrateHandler = new AsyncCallbackJsonWebHandler(
        "/api/calibrate",
        [&ctx](AsyncWebServerRequest *request, JsonVariant &json) {
            JsonObject body = json.as<JsonObject>();

            xSemaphoreTake(ctx.mutex, portMAX_DELAY);
            if (body["dry"].is<int>()) {
                CalibrationStore::saveDry(*ctx.calibration, body["dry"].as<int>());
            }
            if (body["wet"].is<int>()) {
                CalibrationStore::saveWet(*ctx.calibration, body["wet"].as<int>());
            }
            int dryRaw = ctx.calibration->dryRaw();
            int wetRaw = ctx.calibration->wetRaw();
            xSemaphoreGive(ctx.mutex);

            JsonDocument resp;
            resp["ok"] = true;
            resp["dryRaw"] = dryRaw;
            resp["wetRaw"] = wetRaw;
            String out;
            serializeJson(resp, out);
            request->send(200, "application/json", out);
        });
    calibrateHandler->setMethod(HTTP_POST);
    server.addHandler(calibrateHandler);

    // Modern phones probe a captive-portal-detection URL right after
    // joining an AP (e.g. /generate_204, /hotspot-detect.html) and may
    // disconnect on a 404, thinking the network is useless. Redirect
    // anything unrecognized to the dashboard so they stay connected.
    server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
}

}
