# ESP32 Moisture Manager

Firmware for an **ESP32-C5** that reads a capacitive soil-moisture sensor, keeps
about 60 days of history, and hosts its own Wi-Fi dashboard. No cloud, no router,
no app: join the device's network and open a web page.

This is the ESP32-C5 version. There is also an
[ESP32-C3 version](https://github.com/ubc-terence/esp32-moisture-manager-c3); the two are
separate projects that started from the same code.

- **Reads every 10 s** using several ADC samples with the outliers trimmed.
- **Dry/wet calibration** from the dashboard, stored on the device. It refuses to
  calibrate until the reading has settled, so you can't capture a value that's
  still drifting.
- **History**: one sample every 10 minutes (8,640 samples, about 60 days), saved to
  flash so it survives resets.
- **Self-hosted dashboard**: current moisture %, raw ADC value and a history chart,
  served from the device over its own Wi-Fi access point.

Tested on one board and one sensor (see below). Reports from other hardware are very
welcome; see [CONTRIBUTING.md](CONTRIBUTING.md).

## Hardware

| Part | Used here |
|---|---|
| Board | Waveshare ESP32-C5-WIFI6-KIT (ESP32-C5, 16 MB flash) |
| Sensor | Capacitive soil-moisture sensor with an analog output, powered at 3.3 V |

Wiring as built:

| Sensor pin | Board pin | Notes |
|---|---|---|
| AOUT | **GPIO4** | ADC1 channel 3 |
| VCC | **3V3** | Always on. Alternatively wire it to **GPIO8**: the firmware switches that pin around each read, which reduces electrode corrosion. |
| GND | **GND** | |

Pins are constants at the top of [`src/main.cpp`](src/main.cpp). More detail, including
how to tell a working sensor from a floating pin, is in [docs/hardware.md](docs/hardware.md).

## Quick start

You need [PlatformIO](https://platformio.org/install) (the VS Code extension or `pip install platformio`).

```sh
git clone https://github.com/ubc-terence/esp32-moisture-manager.git
cd esp32-moisture-manager

pio test -e native                          # unit tests, no board needed
pio run                                     # build the firmware

# With the board plugged in (find its port with `pio device list`):
pio run -t upload   --upload-port <port>    # flash the firmware
pio run -t uploadfs --upload-port <port>    # flash the dashboard page
```

> **Heads-up:** `uploadfs` replaces the whole filesystem, which also holds the saved
> history. See [docs/development.md](docs/development.md) for how to keep it.

Then:

1. Join the Wi-Fi network **`MoistureManager`** (default password `plant1234`, see
   [Configuration](#configuration)).
2. Open **http://192.168.4.1**.
3. Give the reading about a minute to settle after power-up before calibrating.

## Calibrating

The reading is turned into a 0-100% value using two points: what the sensor reads dry,
and what it reads wet.

1. Hold the probe in dry air and wait until the dashboard stops saying the reading is
   still settling. Click **Set Dry**.
2. Dip the probe to its line in water and wait again. Click **Set Wet**.

The values are stored on the device and survive resets. Until you calibrate, the firmware
uses generic defaults (dry 3000, wet 1200) that will not match your sensor. As a reference,
on the author's board a dry probe read about 1920 and a probe in water about 570, and the
sensor takes roughly 40-70 s to settle after a reset or a big change.

You can also set the values over USB with [`tools/set_calibration.py`](tools/set_calibration.py).

## Configuration

Copy `include/config.local.example.h` to `include/config.local.h` (git-ignored) and set
your own Wi-Fi name and password:

```cpp
#define MM_AP_SSID "MyMoistureManager"
#define MM_AP_PASSWORD "a-better-password"   // 8-63 characters (WPA2)
```

The defaults in [`include/config.h`](include/config.h) are public, so change the password
for any device you actually deploy.

## Project layout

| Path | What |
|---|---|
| `src/main.cpp` | Setup, read loop, Wi-Fi access point |
| `lib/` | Calibration, history, stability tracking, sensor reading, web server |
| `data/index.html` | The dashboard page (served from the device's filesystem) |
| `test/` | Host-side unit tests (Unity) |
| `tools/` | Serial capture, USB calibration, and hardware diagnostic sketches |
| `docs/` | Hardware notes and development guide |

## Known issues

- **Pinned platform.** ESP32-C5 needs the community [pioarduino](https://github.com/pioarduino/platform-espressif32)
  fork, pinned in `platformio.ini`. On Arduino-ESP32 3.3.8+ the soft-AP can start but be invisible
  to clients; `main.cpp` contains a workaround that works on the tested board.
- **PSRAM does not initialize** on the tested board and the boot log prints a harmless
  `MSPI Timing ... PSRAM memory barrier` error. The firmware only uses internal RAM.
- **Little flash headroom.** The firmware uses about 88% of the default app partition.

See [docs/development.md](docs/development.md) for troubleshooting.

## Contributing

Issues and pull requests are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md), which also
explains how the repo is set up for [Claude Code](https://claude.com/claude-code) and other
coding agents.

## License

[MIT](LICENSE)
