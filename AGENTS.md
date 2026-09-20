# ESP32 Moisture Manager: agent guide

ESP32-C5 firmware (Arduino via PlatformIO/pioarduino) that reads a capacitive soil-moisture sensor
and serves a dashboard from its own Wi-Fi access point. Human docs: `README.md`,
`docs/hardware.md`, `docs/development.md`. This file holds only what you can't get from the code.

## Commands

- Unit tests (host only, no board): `pio test -e native`. Always pass `-e native`.
- Build: `pio run` (builds only the firmware; diagnostics are opt-in `diag-*` envs).
- Serial output, non-interactive: `python tools/capture_serial.py --seconds 30 [--reset]`.
  Do not use `pio device monitor`; it never exits.
- Flash: `pio run -t upload --upload-port <port>`. Dashboard: `pio run -t uploadfs ...`.
  **`uploadfs` wipes the whole filesystem, including the saved history.** See `docs/development.md`.

## Gotchas

- **Hardware is the user's.** Flashing, `uploadfs` and NVS writes change a real device. Ask before
  doing any of them, and never run erase commands (`erase_flash`, `pio run -t erase`).
  Verify with `pio test -e native` and `pio run` first; only flash when the user agrees.
- **Build fails with `opening dependency file ... .pio/build/...`?** That is two PlatformIO Core
  versions colliding (a pip `pio` vs the VS Code extension's core), not a code error. See
  `docs/development.md`.
- **Sensor reads look plausible but are meaningless if the ADC pin floats.** A dry-to-wet span of
  only tens of counts, or a value that drains toward 0 and stays there whatever the probe touches,
  means AOUT isn't driven. (Starting high and falling after a reset is normal for a working sensor.)
  Run `diag-pin-probe` (see `docs/hardware.md`) before touching the firmware.
- **Readings need ~40-70 s to settle** after reset or a big change. `stable=0` in that window is
  expected, and calibration is deliberately refused then.
- **Pinned platform.** `platformio.ini` pins pioarduino `55.03.311`. On Arduino-ESP32 3.3.8+ the
  soft-AP can be invisible to clients; `main.cpp` has a workaround. Re-check AP visibility on a real
  device after any platform change.
- **PSRAM doesn't initialize** and the boot log shows an `MSPI Timing ... PSRAM` error. Harmless.
- **Flash is ~88% full** (default 1.25 MB app partition). Check the size line after adding code.
- **Secrets.** `include/config.local.h` holds the user's Wi-Fi password and is git-ignored. Don't
  read, print or commit it. Deploy-time settings go in `include/config.h` defaults or that override.

## Conventions

- Pure logic (`Calibration`, `HistoryBuffer`, `ReadingStability`) has no Arduino dependencies and is
  unit-tested on the host. New logic should be too; put Arduino-specific code in `src/` and the thin
  wrappers.
- Comments explain *why* or record a measurement ("noise spans ~5 counts"), not what the code does.
- Commit messages: imperative subject line, then a body explaining why.
- When the change affects behavior on hardware, say in the PR whether you tested on a board.
