# Hardware notes

Everything here was measured on one Waveshare ESP32-C5-WIFI6-KIT and one capacitive
soil-moisture sensor. Treat the numbers as a reference, not a spec.

## Board

- Chip: ESP32-C5 rev v1.0, 16 MB flash (confirmed with `esptool flash-id`), USB-Serial/JTAG.
- PlatformIO board definition: `esp32-c5-devkitc-1`, with `board_build.flash_size = 16MB`.
- **PSRAM does not initialize.** `psramFound()` is 0 and PSRAM allocations fail, with the
  boot-log line `E MSPI Timing: Failed to allocate dummy cacheline for PSRAM memory barrier!`.
  It is harmless: the firmware only uses internal RAM (about 15% of ~320 KB). We could not tell
  whether the module has no PSRAM or the prebuilt Arduino libraries just can't bring it up.
  `pio run -e diag-psram-check -t upload` re-tests it.
- The default (4 MB-style) partition table is in use: app partitions are 1.25 MB each and the
  firmware fills about 88% of one. Adding features may need a custom partition table.

## Wiring

| Sensor pin | Board pin |
|---|---|
| AOUT | GPIO4 (ADC1 channel 3) |
| VCC | 3V3, or GPIO8 for power-cycled reads |
| GND | GND |

GPIO1-GPIO6 all read as ADC inputs in our tests; GPIO4 was chosen because it is not a strapping
pin and is not used for flash, USB or UART0.

**Power-cycling.** `SensorReader` drives `VCC_PIN` (GPIO8) high before each read and low after.
With VCC on 3V3 that pin is unconnected and the toggling is a harmless no-op. Wiring the sensor's VCC
to GPIO8 powers it only while reading, which reduces electrode corrosion, at the cost of the
sensor having less time to settle.

## What a working sensor looks like

Measured with the probe dry in air vs. in water (12-bit ADC, default attenuation):

| State | Raw ADC | Noise |
|---|---|---|
| Dry, in air | ~1920 | ±3 counts |
| In water | ~570 | ±3 counts |

That is a span of roughly 1350 counts, so 1 count is about 0.07% of the range.

**Settling.** The sensor's output pulls down slowly and rises quickly:

- After a reset the pin starts charged (~3.2 V, raw ~2000+) and takes about 40 s to fall to the
  wet level, or ~70 s to reach a stable dry reading.
- Going dry to wet takes about 40-60 s to settle; going wet to dry is fast.
- The firmware's stability check (last 3 readings within 15 counts) and the dashboard's
  "still settling" message exist because of this. Don't calibrate, or trust a reading, right
  after a reset or a big change.

## Floating pin: the failure that looks like a working sensor

If nothing is actually driving the ADC pin (AOUT on the wrong pin, no sensor ground, a loose
jumper), it floats. A floating pin still produces plausible-looking numbers: it holds stray
charge, drifts, and even changes when you touch the probe or dip it in water. We lost time to this.

Signs of a floating pin:

- Tiny, noisy span: a dry-to-wet change of only tens of counts (we saw ~80).
- The value drains steadily toward 0 (about -27 mV/s in our case) and stays there, whatever the probe
  touches. A working sensor also starts high after a reset (see "Settling"), but it then settles at a
  level that depends on the probe: dry and wet end up far apart.
- The value ignores GPIO8 switching the sensor's power.

The definitive test is the **force-and-release probe**: drive the pin to 0 V, let go, and see
whether something pulls it back.

```sh
pio run -e diag-pin-probe -t upload --upload-port <port>
python tools/capture_serial.py --seconds 40 --reset
pio run -t upload --upload-port <port>     # put the real firmware back
```

It prints `VERDICT: DRIVEN` (something drives the pin; expected for a wired sensor) or
`VERDICT: FLOATING`. A driven sensor output recovers to its own level within a few hundred
milliseconds of being forced low; a floating pin stays near zero. `DRIVEN` does not prove it is
the *right* signal, so also confirm the raw value changes between dry and wet.

Other things to check when the verdict is `FLOATING`: the AOUT wire is on the pin you think it is
(the header labels), the sensor's GND is connected, VCC is powered, and jumpers are seated. A
multimeter on AOUT with the sensor powered should read roughly 1-3 V.

## Sensor and ADC notes

- 12-bit ADC (0-4095). The Arduino default attenuation (11 dB) is used; `SensorReader` does not
  change it.
- Each reading is 8 samples 5 ms apart with the highest and lowest dropped, averaged.
- The default calibration (dry 3000, wet 1200) came from an earlier ESP32-C3 build with a different
  sensor and does not match the C5 numbers above. Always calibrate.
