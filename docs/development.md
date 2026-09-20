# Development guide

## Setup

Install [PlatformIO](https://platformio.org/install): either the VS Code extension or
`pip install platformio`. The first build downloads the toolchain and the pinned
[pioarduino](https://github.com/pioarduino/platform-espressif32) platform (about 1 GB).

## Commands

| Task | Command |
|---|---|
| Unit tests (host, no board) | `pio test -e native` |
| Build the firmware | `pio run` |
| Flash the firmware | `pio run -t upload --upload-port <port>` |
| Build the dashboard filesystem image | `pio run -t buildfs` |
| Flash the dashboard (**wipes history**) | `pio run -t uploadfs --upload-port <port>` |
| List serial ports | `pio device list` |
| Capture serial output and exit | `python tools/capture_serial.py --seconds 30 [--reset]` |

Always pass `-e native` to `pio test`. Plain `pio run` builds only the firmware
(`default_envs` in `platformio.ini`), so `pio test` without `-e` would target the board.

`pio device monitor` never exits, which makes it awkward in scripts and coding-agent sessions;
use `tools/capture_serial.py` there.

## Code layout and testing

- `lib/Calibration`, `lib/HistoryBuffer`, `lib/ReadingStability` are pure logic with no Arduino
  dependencies and are covered by the `native` unit tests. Keep new logic testable this way and
  put Arduino-specific code in `src/` and the thin wrappers (`SensorReader`, `WebServer`, the
  `*Store` classes).
- `data/index.html` is a single self-contained page. It is not built or bundled; edit it directly.
- Build-time settings (Wi-Fi name and password) live in `include/config.h`, overridable in the
  git-ignored `include/config.local.h`.

## Storage

| What | Where | Notes |
|---|---|---|
| Calibration (`dry`, `wet`) | NVS, namespace `cal`, int32 keys | Written by the dashboard or `tools/set_calibration.py` |
| History | LittleFS `history.bin` | One sample per 10 min, 8,640 samples max |
| Dashboard page | LittleFS `index.html` | Uploaded with `uploadfs` |

Partition table (default layout): `nvs` 0x9000 (0x5000), `app0` 0x10000 (0x140000),
`spiffs` (LittleFS) 0x290000 (0x160000).

### Redeploying the dashboard without losing history

`uploadfs` rewrites the whole LittleFS partition, including `history.bin`. To keep the history:

1. Read the partition off the board and pull the file out (needs `pip install littlefs-python`):
   ```sh
   python -m esptool --port <port> read-flash 0x290000 0x160000 fs_dump.bin
   python - <<'EOF'
   from littlefs import LittleFS
   img = open("fs_dump.bin", "rb").read()
   fs = LittleFS(block_size=4096, block_count=len(img) // 4096, mount=False)
   fs.context.buffer[:] = img
   fs.mount()
   open("data/history.bin", "wb").write(fs.open("history.bin", "rb").read())
   EOF
   ```
2. `pio run -t uploadfs --upload-port <port>` (the file in `data/` is included in the image).
3. `rm data/history.bin` (it is git-ignored, but don't leave it lying around).

## Diagnostics

Sketches in `tools/diagnostics/` are opt-in build environments. Flashing one **replaces the
firmware** (calibration and history are untouched); flash the normal firmware again afterwards.

| Env | Purpose |
|---|---|
| `diag-pin-probe` | Is the sensor's analog pin driven, or floating? See [hardware.md](hardware.md) |
| `diag-psram-check` | Does PSRAM initialize in this build? |

## Troubleshooting

### `fatal error: opening dependency file .pio/build/.../*.d: No such file or directory`

Not a code error. The build directory is deleted while the build is running. We traced this to
**two PlatformIO Core versions sharing `~/.platformio`**: a `pip`-installed `pio` and the VS Code
PlatformIO extension's own bundled core. They disagree on which `tool-scons` version to install
and keep swapping packages; the extension reacts by starting its own background build in the same
project, which wipes `.pio/build` under your build. It mostly hits full rebuilds and the first
build after editing `platformio.ini`.

Fixes, any one of which works:

- Build with the extension's own core: `~/.platformio/penv/bin/python -m platformio run` (on
  macOS/Linux; the path differs on Windows).
- Use only one PlatformIO: uninstall the `pip` one, or always build from the extension.
- Turn off the extension's setting `platformio-ide.autoRebuildAutocompleteIndex`.

### The Wi-Fi network doesn't appear

Arduino-ESP32 3.3.8+ has a regression where `softAP()` reports success but clients can't see the
network. `main.cpp` resets Wi-Fi before starting the AP as a workaround, and it works on the tested
board. If you change the platform version, re-check that the network is visible from a phone.

### Readings are noisy, drift, or barely change between dry and wet

Probably a floating pin. See [hardware.md](hardware.md) and run `diag-pin-probe`.

### Dashboard says "reading is still settling" for a long time

Expected for 40-70 s after a reset or when you move the probe between air and water. If it never
clears with the probe held still, capture the serial output and look at how much `raw` jumps
between readings: the stability check needs the last 3 readings within 15 counts.

## Updating the platform

`platformio.ini` pins the pioarduino release (`55.03.311`, Arduino-ESP32 3.3.11). To try a newer
one, change the URL, then run the tests, build, flash a board, and confirm the Wi-Fi network is
visible and the dashboard works before merging.

## CI

GitHub Actions (`.github/workflows/ci.yml`) runs the native tests, builds the firmware, and builds
the filesystem image on every push and pull request. It cannot flash or test on hardware.
