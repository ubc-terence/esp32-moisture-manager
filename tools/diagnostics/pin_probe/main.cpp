// Pin probe: is the sensor's analog output pin actually being DRIVEN by a
// sensor, or is it FLOATING (nothing connected / wrong pin / no ground)?
//
// A floating ADC pin is easy to mistake for a working sensor: it holds stray
// charge, drifts slowly, and even changes when you touch the probe or dip it in
// water, so the readings look plausible but are meaningless.
//
// Method: drive the pin LOW for 300 ms, release it, and watch how it recovers.
//   * A driven sensor output pulls the pin back to its own level within
//     milliseconds to a few hundred milliseconds.
//   * A floating pin stays near 0 (creeping up only a few counts).
//
// Build/flash: pio run -e diag-pin-probe -t upload --upload-port <port>
// Read output: python tools/capture_serial.py --seconds 40 --reset
// Afterwards re-flash the real firmware: pio run -t upload --upload-port <port>
//
// Pins can be overridden at build time: -D PROBE_PIN=4 -D POWER_PIN=8
// Set POWER_PIN to -1 if the sensor's VCC is wired straight to 3V3.

#include <Arduino.h>

#ifndef PROBE_PIN
#define PROBE_PIN 4
#endif
#ifndef POWER_PIN
#define POWER_PIN 8
#endif

static const uint32_t SAMPLE_TIMES_MS[] = {0, 5, 20, 50, 100, 200, 500, 1000, 2000};
static const int N_SAMPLES = sizeof(SAMPLE_TIMES_MS) / sizeof(SAMPLE_TIMES_MS[0]);
static const int IDX_500_MS = 6;  // index of the 500 ms sample in SAMPLE_TIMES_MS

static int meanRead(int samples) {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(PROBE_PIN);
        delay(3);
    }
    return sum / samples;
}

// Drives the pin to `level` for 300 ms, releases it, then records the ADC at
// each SAMPLE_TIMES_MS offset into out[].
static void releaseAndTrace(int level, int out[]) {
    pinMode(PROBE_PIN, OUTPUT);
    digitalWrite(PROBE_PIN, level);
    delay(300);
    pinMode(PROBE_PIN, INPUT);
    uint32_t t0 = micros();
    for (int i = 0; i < N_SAMPLES; i++) {
        while ((micros() - t0) < SAMPLE_TIMES_MS[i] * 1000UL) {
        }
        out[i] = analogRead(PROBE_PIN);
    }
}

static void printTrace(const char *label, const int v[]) {
    Serial.printf("  %-22s", label);
    for (int i = 0; i < N_SAMPLES; i++) {
        Serial.printf(" %ums:%d", (unsigned)SAMPLE_TIMES_MS[i], v[i]);
    }
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    delay(2500);  // let USB-CDC enumerate so the first lines aren't lost

    Serial.printf("PIN_PROBE start: PROBE_PIN=GPIO%d POWER_PIN=%d\n", PROBE_PIN, POWER_PIN);
    if (POWER_PIN >= 0) {
        pinMode(POWER_PIN, OUTPUT);
        digitalWrite(POWER_PIN, HIGH);  // power the sensor if it is switched by a GPIO
        delay(500);
    }
    pinMode(PROBE_PIN, INPUT);

    int idleNow = meanRead(8);
    delay(5000);
    int idleLater = meanRead(8);
    // Just after a reset even a working sensor drifts while it settles (the pin starts charged), so the
    // idle drift is context only; the force-and-release verdict below is the real test.
    Serial.printf("  idle level: %d now, %d after 5 s (context only; see the verdict)\n", idleNow, idleLater);

    int afterLow[N_SAMPLES], afterHigh[N_SAMPLES];
    releaseAndTrace(LOW, afterLow);
    Serial.println("  ADC counts (0-4095) after release:");
    printTrace("after driving LOW:", afterLow);
    delay(3000);
    releaseAndTrace(HIGH, afterHigh);
    printTrace("after driving HIGH:", afterHigh);

    bool driven = afterLow[IDX_500_MS] >= 300;
    Serial.println();
    if (driven) {
        Serial.printf("VERDICT: DRIVEN. GPIO%d recovered to %d counts within 500 ms of being forced low, so\n"
                      "         something is actively driving it (expected for a connected sensor).\n",
                      PROBE_PIN, afterLow[IDX_500_MS]);
        Serial.println("         This does not prove it is the RIGHT signal; check the value changes dry vs wet.");
    } else {
        Serial.printf("VERDICT: FLOATING. GPIO%d only reached %d counts 500 ms after being forced low.\n"
                      "         Nothing is driving this pin. Check AOUT wiring, sensor GND and VCC, and the pin\n"
                      "         number. (False alarm only if the sensor really outputs ~0 V right now.)\n",
                      PROBE_PIN, afterLow[IDX_500_MS]);
    }
    Serial.println("PIN_PROBE done");
    if (POWER_PIN >= 0) digitalWrite(POWER_PIN, LOW);
}

void loop() {
    delay(1000);
}
