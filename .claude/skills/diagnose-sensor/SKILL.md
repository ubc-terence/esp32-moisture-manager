---
name: diagnose-sensor
description: "Diagnose suspicious moisture-sensor readings on the ESP32 Moisture Manager (noisy, drifting, tiny dry-to-wet change, or moisture stuck at 0% or 100%). Tells a working sensor from a floating ADC pin with the force-and-release probe."
---

# Diagnose the sensor

A floating ADC pin (AOUT not actually driven) produces plausible-looking numbers that even respond to
touching the probe. Rule this out **before** changing the firmware or calibration. Background and what a
healthy sensor looks like: `docs/hardware.md`.

## 1. Look at the readings

```sh
python tools/capture_serial.py --reset --seconds 90
```

Red flags: a dry-to-wet change of only tens of counts; raw values that drain toward 0 and stay there
whatever the probe touches. (Starting around 2000+ and falling for the first ~40-70 s after a reset is
normal; what matters is whether it settles at a probe-dependent level.)

Healthy reference (author's board): dry ~1920, wet ~570, noise ±3 counts, settles in ~40-70 s.

## 2. Run the pin probe (this replaces the firmware on the board)

Flashing a diagnostic sketch overwrites the running firmware (calibration and history are kept). Ask the
user first, and re-flash the normal firmware afterwards.

```sh
pio run -e diag-pin-probe -t upload --upload-port <port>
python tools/capture_serial.py --reset --seconds 40 --until "PIN_PROBE done"
pio run -t upload --upload-port <port>      # restore the real firmware
```

Pins default to `PROBE_PIN=4`, `POWER_PIN=8`. Override them without editing files, e.g. for a sensor on
GPIO5 powered from 3V3:
`PLATFORMIO_BUILD_FLAGS="-D PROBE_PIN=5 -D POWER_PIN=-1" pio run -e diag-pin-probe -t upload --upload-port <port>`

## 3. Interpret

- `VERDICT: FLOATING`: nothing is driving the pin. Have the user check that AOUT is on the pin they
  think it is (header labels), that the sensor's GND and VCC are connected, and that jumpers are seated.
  A multimeter on AOUT with the sensor powered should read about 1-3 V. Don't touch the firmware yet.
- `VERDICT: DRIVEN`: a signal is present, but it isn't proven to be the right one. Confirm the raw value
  changes between dry and wet, then calibrate (dashboard or `tools/set_calibration.py`).

Report what you observed and what you could not check.
