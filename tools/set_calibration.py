#!/usr/bin/env python3
"""Set the sensor's dry/wet calibration on the board over USB (no Wi-Fi needed).

The dashboard's "Set Dry" / "Set Wet" buttons are the normal way to calibrate.
Use this when you already know the raw values (e.g. from `tools/capture_serial.py`)
or can't reach the dashboard.

By default this only PREPARES the NVS image and shows the plan. Nothing touches the
board until you pass --write.

  python tools/set_calibration.py --dry 1920 --wet 566             # dry run
  python tools/set_calibration.py --dry 1920 --wet 566 --write     # flash it

What --write does:
  1. reads the board's partition table to find the NVS partition,
  2. backs up the current NVS partition to --backup-dir,
  3. generates a fresh NVS image containing only the calibration (namespace "cal",
     int32 keys "dry" and "wet", exactly what CalibrationStore reads),
  4. flashes it and resets the board.

WARNING: this REPLACES the whole NVS partition. Anything else stored there (Wi-Fi/PHY
calibration data, other settings) is dropped; the ESP32 regenerates its own on next
boot. The LittleFS history and the firmware are not touched.

Needs esptool and esp-idf-nvs-partition-gen (`pip install -r tools/requirements.txt`).
"""
import argparse
import os
import struct
import subprocess
import sys
import tempfile
import time

PARTITION_TABLE_OFFSET = 0x8000
PARTITION_TABLE_SIZE = 0xC00


def run(cmd, **kw):
    return subprocess.run(cmd, check=True, **kw)


def cmd_name(name):
    """esptool 5.x renamed read_flash -> read-flash (old names still work but warn); 4.x needs underscores."""
    try:
        import esptool

        major = int(esptool.__version__.split(".")[0])
    except Exception:
        major = 4
    return name.replace("_", "-") if major >= 5 else name


def esptool(port, command, *args, **kw):
    return run([sys.executable, "-m", "esptool", "--port", port, cmd_name(command), *args], **kw)


def find_port():
    try:
        from serial.tools import list_ports
    except ImportError:
        sys.exit("pyserial is required: pip install -r tools/requirements.txt (or pass --port)")
    ports = [p.device for p in list_ports.comports() if p.vid == 0x303A]
    if len(ports) != 1:
        sys.exit("Expected exactly one Espressif USB device, found %d. Pass --port." % len(ports))
    return ports[0]


def find_nvs(port, workdir):
    table = os.path.join(workdir, "ptable.bin")
    esptool(port, "read_flash", hex(PARTITION_TABLE_OFFSET), hex(PARTITION_TABLE_SIZE), table, stdout=subprocess.DEVNULL)
    data = open(table, "rb").read()
    for i in range(0, len(data) - 31, 32):
        entry = data[i : i + 32]
        if entry[:2] != b"\xaa\x50":
            break
        ptype, subtype, offset, size = struct.unpack_from("<BBII", entry, 2)
        name = entry[12:28].split(b"\0")[0].decode(errors="replace")
        if ptype == 1 and subtype == 2 and name == "nvs":
            return offset, size
    sys.exit("No 'nvs' partition found in the board's partition table.")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--dry", type=int, required=True, help="raw ADC value with the probe dry / in air")
    ap.add_argument("--wet", type=int, required=True, help="raw ADC value with the probe in water")
    ap.add_argument("--port", help="serial port (default: auto-detect)")
    ap.add_argument("--write", action="store_true", help="actually flash the board (default: prepare only)")
    ap.add_argument("--backup-dir", default=".nvs-backups", help="where to save the old NVS partition (default: ./.nvs-backups)")
    args = ap.parse_args()

    for name, v in (("dry", args.dry), ("wet", args.wet)):
        if not 0 <= v <= 4095:
            sys.exit("--%s must be a raw 12-bit ADC value (0-4095), got %d" % (name, v))
    if args.dry == args.wet:
        sys.exit("--dry and --wet must differ")

    with tempfile.TemporaryDirectory() as work:
        csv = os.path.join(work, "cal.csv")
        with open(csv, "w") as f:
            f.write("key,type,encoding,value\ncal,namespace,,\ndry,data,i32,%d\nwet,data,i32,%d\n" % (args.dry, args.wet))

        if not args.write:
            offset, size = 0x9000, 0x5000  # default partition table; confirmed against the board when --write is used
            print("DRY RUN (nothing will be written). Assuming the default NVS layout: offset 0x9000, size 0x5000.")
        else:
            port = args.port or find_port()
            offset, size = find_nvs(port, work)
            print("NVS partition on the board: offset %#x, size %#x" % (offset, size))

        image = os.path.join(work, "nvs_cal.bin")
        run([sys.executable, "-m", "esp_idf_nvs_partition_gen", "generate", csv, image, hex(size)], stdout=subprocess.DEVNULL)
        print("Generated NVS image: cal/dry=%d  cal/wet=%d  (%d bytes)" % (args.dry, args.wet, os.path.getsize(image)))

        if not args.write:
            print("Re-run with --write to flash it (this replaces the whole NVS partition).")
            return

        os.makedirs(args.backup_dir, exist_ok=True)
        backup = os.path.join(args.backup_dir, "nvs_%s.bin" % time.strftime("%Y%m%d-%H%M%S"))
        esptool(port, "read_flash", hex(offset), hex(size), backup, stdout=subprocess.DEVNULL)
        print("Backed up the old NVS partition to %s" % backup)

        esptool(port, "write_flash", hex(offset), image)
        print("Done. The board has reset; calibration is dry=%d wet=%d." % (args.dry, args.wet))
        print("To restore the previous NVS: python -m esptool --port %s %s %#x %s" % (port, cmd_name("write_flash"), offset, backup))


if __name__ == "__main__":
    main()
