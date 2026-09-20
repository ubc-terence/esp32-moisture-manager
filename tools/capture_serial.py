#!/usr/bin/env python3
"""Capture the board's serial output for a fixed time, then exit.

A non-interactive alternative to `pio device monitor` (which never exits), so it
works in scripts, CI-style checks and coding-agent sessions.

Examples:
  python tools/capture_serial.py --seconds 30
  python tools/capture_serial.py --seconds 60 --reset          # catch the boot log
  python tools/capture_serial.py --until "stable=1" --seconds 180

Needs pyserial (`pip install -r tools/requirements.txt`; PlatformIO's own Python
already has it).
"""
import argparse
import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    sys.exit("pyserial is required: pip install -r tools/requirements.txt")

ESPRESSIF_USB_VID = 0x303A  # native USB-Serial/JTAG on ESP32-C3/C5/S3/...


def find_port():
    ports = [p.device for p in list_ports.comports() if p.vid == ESPRESSIF_USB_VID]
    if not ports:
        sys.exit("No Espressif USB serial device found. Plug the board in or pass --port.")
    if len(ports) > 1:
        sys.exit("Several Espressif devices found (%s). Pass --port." % ", ".join(ports))
    return ports[0]


def open_port(port, baud, wait_s=15):
    """Open the port, retrying while the USB device re-enumerates after a reset/flash."""
    deadline = time.time() + wait_s
    while True:
        try:
            return serial.Serial(port, baud, timeout=0.3)
        except (serial.SerialException, OSError):
            if time.time() > deadline:
                sys.exit("Could not open %s" % port)
            time.sleep(0.5)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--port", help="serial port (default: auto-detect the Espressif USB device)")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--seconds", type=float, default=30, help="how long to capture (default 30)")
    ap.add_argument("--reset", action="store_true", help="pulse RTS to reset the board first, to catch the boot log")
    ap.add_argument("--until", help="stop early once a line contains this text")
    args = ap.parse_args()

    port = args.port or find_port()
    s = open_port(port, args.baud)
    if args.reset:
        s.dtr = False
        s.rts = True
        time.sleep(0.2)
        s.rts = False

    t0 = time.time()
    buf = b""
    try:
        while time.time() - t0 < args.seconds:
            chunk = s.read(256)
            if not chunk:
                continue
            buf += chunk
            while b"\n" in buf:
                raw, buf = buf.split(b"\n", 1)
                line = raw.decode(errors="replace").rstrip()
                print("%6.1fs %s" % (time.time() - t0, line), flush=True)
                if args.until and args.until in line:
                    return
    finally:
        s.close()


if __name__ == "__main__":
    main()
