---
name: verify-firmware
description: "Verify a change to the ESP32 Moisture Manager firmware or dashboard: run the native unit tests, build, and (only if the user agrees) flash a connected board and check the serial output. Use before saying firmware work is done."
---

# Verify firmware

Evidence before claims: run each step and read its output before saying anything passes.

## 1. Always (no hardware needed)

```sh
pio test -e native      # expect: "N test cases: N succeeded"
pio run                 # expect SUCCESS; note the "Flash:" line (the app partition is ~88% full)
```

If the build fails with `opening dependency file ... .pio/build/...`, it is a PlatformIO Core collision, not
your change. See "Troubleshooting" in `docs/development.md`, then rebuild.

If `data/index.html` changed, also run `pio run -t buildfs` to confirm the filesystem image builds.

## 2. On hardware (ask the user first)

Flashing changes a real device. Confirm a board is attached (`pio device list`) and that the user wants
it flashed. Never erase flash.

```sh
pio run -t upload --upload-port <port>
python tools/capture_serial.py --reset --seconds 90 --until "stable=1"
```

Check the output for:

- `AP SSID: <name>` and `AP IP address: 192.168.4.1` (the access point came up).
- `raw=... moisture=...% stable=...` lines every ~10 s.
- `stable=0` for the first 40-70 s after a reset is normal; it should reach `stable=1` with the probe still.
- The boot line `E ... MSPI Timing ... PSRAM memory barrier` and `STA disconnect failed 0x3001` are known and harmless.

If the dashboard changed, `uploadfs` is needed and it **wipes the saved history**; follow "Redeploying the
dashboard without losing history" in `docs/development.md` and get the user's agreement first.

## 3. Report

State exactly what ran and what it showed. Say plainly what you did **not** verify (for example: not
flashed, Wi-Fi network visibility not checked from a real device, dashboard not opened in a browser).
