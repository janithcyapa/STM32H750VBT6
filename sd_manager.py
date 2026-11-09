#!/usr/bin/env python3
"""
STM32H750 MicroSD Telemetry CSV Manager
---------------------------------------
Command-line and interactive terminal utility to list, download, and batch-download
CSV telemetry log files directly over the USB Virtual COM port (CDC).

Usage:
  Interactive Menu:
    python3 sd_manager.py

  CLI Commands:
    python3 sd_manager.py --list
    python3 sd_manager.py --get LOG0001.CSV
    python3 sd_manager.py --all
    python3 sd_manager.py --port /dev/ttyACM0 --out ./my_data/
"""

import sys
import os
import time
import argparse
import glob
from pathlib import Path

try:
    import serial
    import serial.tools.list_ports
except ImportError:
    print("[ERROR] 'pyserial' is not installed. Please run: pip install pyserial")
    sys.exit(1)


def find_default_port():
    """Auto-detect STM32 USB CDC device port across Linux, macOS, and Windows."""
    ports = list(serial.tools.list_ports.comports())
    
    # Priority matching by STM32 USB CDC (VID 0483, PID 5740)
    for p in ports:
        if p.vid == 0x0483 and p.pid == 0x5740:
            return p.device

    for p in ports:
        if p.vid == 0x0483:
            return p.device
        if any(keyword in (p.description or "").lower() for keyword in ["stm32", "cdc", "vcp"]):
            return p.device
            
    # Platform fallback
    if sys.platform.startswith("linux"):
        matches = glob.glob("/dev/ttyACM*")
        if matches:
            return sorted(matches)[0]
    elif sys.platform == "darwin":
        matches = glob.glob("/dev/cu.usbmodem*")
        if matches:
            return sorted(matches)[0]
            
    return ports[0].device if ports else None


class STM32SDManager:
    def __init__(self, port, baudrate=115200, timeout=2.0):
        self.port = port
        self.baudrate = baudrate
        self.timeout = timeout
        self.ser = None

    def connect(self):
        try:
            self.ser = serial.Serial(self.port, self.baudrate, timeout=self.timeout)
            # Flush existing unread serial noise
            time.sleep(0.1)
            self.ser.reset_input_buffer()
            return True
        except Exception as e:
            print(f"[!] Connection error on {self.port}: {e}")
            return False

    def close(self):
        if self.ser and self.ser.is_open:
            self.ser.close()

    def send_cmd(self, cmd):
        self.ser.reset_input_buffer()
        self.ser.write((cmd.strip() + "\r\n").encode("utf-8"))
        self.ser.flush()

    def get_status(self):
        self.send_cmd("STATUS")
        start = time.time()
        while time.time() - start < 1.5:
            line = self.ser.readline().decode("utf-8", errors="replace").strip()
            if line.startswith("[STATUS:"):
                return line[8:-1]
        return "NO_RESPONSE"

    def list_files(self):
        """Query the STM32 for all CSV files on the SD card."""
        self.send_cmd("LIST")
        files = []
        started = False
        start_time = time.time()

        while time.time() - start_time < 3.0:
            raw = self.ser.readline()
            if not raw:
                continue
            line = raw.decode("utf-8", errors="replace").strip()

            if line == "[BUSY_RECORDING]":
                print("\n[!] STM32 is currently recording. Press Button K2 on the board to stop recording first.")
                return []
            if line == "[ERR_MOUNT_FAILED]":
                print("\n[!] SD Card mount failed. Please check that FAT32 card is inserted.")
                return []
            if line == "[LIST_BEGIN]":
                started = True
                continue
            if line == "[LIST_END]":
                break

            if started and line.startswith("FILE:"):
                parts = line[5:].split(",")
                if len(parts) >= 2:
                    try:
                        fname = parts[0].strip()
                        fsize = int(parts[1].strip())
                        files.append({"name": fname, "size": fsize})
                    except ValueError:
                        pass

        # Sort files by name (e.g. LOG0001.CSV, LOG0002.CSV)
        files.sort(key=lambda x: x["name"])
        return files

    def download_file(self, filename, output_dir="./downloads"):
        """Download a single CSV file over USB CDC with live progress."""
        os.makedirs(output_dir, exist_ok=True)
        dest_path = Path(output_dir) / filename

        self.send_cmd(f"GET {filename}")
        start_time = time.time()
        file_size = None
        started = False

        # Wait for START_FILE response
        while time.time() - start_time < 3.0:
            line = self.ser.readline().decode("utf-8", errors="replace").strip()
            if line.startswith("[BUSY_RECORDING]"):
                print("[!] STM32 is currently recording. Stop recording first.")
                return False
            if line.startswith(f"[ERR_NOT_FOUND:{filename}]"):
                print(f"[!] File '{filename}' not found on SD card.")
                return False
            if line.startswith(f"[START_FILE:{filename},"):
                try:
                    file_size = int(line.split(",")[1].rstrip("]"))
                    started = True
                    break
                except (IndexError, ValueError):
                    pass

        if not started or file_size is None:
            print(f"[!] Timeout waiting for transfer of '{filename}'.")
            return False

        # Read the file content
        print(f"[*] Downloading '{filename}' ({file_size:,} bytes)...")
        received_bytes = bytearray()
        last_progress_time = time.time()
        transfer_start = time.time()

        while len(received_bytes) < file_size and (time.time() - last_progress_time < 4.0):
            to_read = min(4096, file_size - len(received_bytes))
            chunk = self.ser.read(to_read)
            if chunk:
                received_bytes.extend(chunk)
                last_progress_time = time.time()

                # Print progress bar
                pct = (len(received_bytes) / file_size) * 100.0
                bar_len = 30
                filled = int(bar_len * len(received_bytes) // file_size)
                bar = "=" * filled + "-" * (bar_len - filled)
                elapsed = max(0.001, time.time() - transfer_start)
                speed_kb = (len(received_bytes) / 1024.0) / elapsed
                sys.stdout.write(f"\r  [{bar}] {pct:5.1f}% ({len(received_bytes):,}/{file_size:,} B) - {speed_kb:.1f} KB/s")
                sys.stdout.flush()

        sys.stdout.write("\n")

        # Wait for END_FILE marker
        end_marker = self.ser.readline().decode("utf-8", errors="replace").strip()

        # Write to disk
        with open(dest_path, "wb") as f:
            f.write(received_bytes)

        if len(received_bytes) == file_size:
            print(f"[+] Successfully saved: {dest_path.resolve()} ({file_size:,} bytes)")
            return True
        else:
            print(f"[!] Warning: Incomplete transfer ({len(received_bytes):,} of {file_size:,} bytes received).")
            return False

    def start_recording(self):
        """Send START command to trigger 10Hz logging to SD card."""
        self.send_cmd("START")
        start = time.time()
        while time.time() - start < 1.5:
            line = self.ser.readline().decode("utf-8", errors="replace").strip()
            if "RECORDING STARTED" in line or "[ALREADY_RECORDING]" in line:
                print(f"[+] {line}")
                return True
            if "ERROR" in line:
                print(f"[!] {line}")
                return False
        print("[!] No confirmation received from STM32.")
        return False

    def stop_recording(self):
        """Send STOP command to commit and finalize SD card CSV file."""
        self.send_cmd("STOP")
        start = time.time()
        while time.time() - start < 2.0:
            line = self.ser.readline().decode("utf-8", errors="replace").strip()
            if "RECORDING STOPPED" in line or "[NOT_RECORDING]" in line:
                print(f"[+] {line}")
                return True
        print("[!] No confirmation received from STM32.")
        return False

    def delete_file(self, filename):
        self.send_cmd(f"DEL {filename}")
        start = time.time()
        while time.time() - start < 2.0:
            line = self.ser.readline().decode("utf-8", errors="replace").strip()
            if line.startswith(f"[OK_DELETED:{filename}]"):
                print(f"[+] File '{filename}' deleted from SD card.")
                return True
            if line.startswith(f"[ERR_DELETE:{filename}]"):
                print(f"[!] Failed to delete '{filename}'.")
                return False
        return False


def print_files_table(files):
    if not files:
        print("\n  [No CSV log files found on SD card]\n")
        return

    print("\n" + "=" * 65)
    print(f" {'#':<3} | {'Filename':<14} | {'Size':<14} | {'Est. Records':<15}")
    print("-" * 65)
    for idx, f in enumerate(files, 1):
        # Average record ~110 bytes
        est_recs = max(0, (f["size"] - 140) // 110)
        size_str = f"{f['size']:,} B"
        print(f" {idx:<3} | {f['name']:<14} | {size_str:<14} | ~{est_recs} records")
    print("=" * 65 + "\n")


def interactive_menu(mgr, output_dir):
    while True:
        print("=" * 55)
        print("    STM32H750 MicroSD Telemetry CSV Manager")
        print("=" * 55)
        print(f"  Port: {mgr.port} (115200 baud)")
        print(f"  Destination: {os.path.abspath(output_dir)}")
        print("-" * 55)
        print("  [1] List all CSV files on SD card")
        print("  [2] Download a specific file")
        print("  [3] Batch download ALL files")
        print("  [4] Delete a file from SD card")
        print("  [5] Check Board Status")
        print("  [6] START Recording (K2 Trigger)")
        print("  [7] STOP Recording (K2 Trigger)")
        print("  [0] Exit")
        print("-" * 55)

        choice = input("Enter option [0-7]: ").strip()

        if choice == "1":
            files = mgr.list_files()
            print_files_table(files)

        elif choice == "2":
            files = mgr.list_files()
            print_files_table(files)
            if not files:
                continue
            sel = input(f"Enter file number (1-{len(files)}) or filename: ").strip()
            target_fname = None
            if sel.isdigit() and 1 <= int(sel) <= len(files):
                target_fname = files[int(sel) - 1]["name"]
            elif any(f["name"].upper() == sel.upper() for f in files):
                target_fname = sel.upper()

            if target_fname:
                mgr.download_file(target_fname, output_dir)
            else:
                print("[!] Invalid selection.")

        elif choice == "3":
            files = mgr.list_files()
            print_files_table(files)
            if not files:
                continue
            confirm = input(f"Download all {len(files)} files to '{output_dir}'? (y/n): ").strip().lower()
            if confirm == "y":
                success_count = 0
                for f in files:
                    if mgr.download_file(f["name"], output_dir):
                        success_count += 1
                    time.sleep(0.05)
                print(f"\n[+] Batch complete: {success_count}/{len(files)} files downloaded successfully.")

        elif choice == "4":
            files = mgr.list_files()
            print_files_table(files)
            if not files:
                continue
            sel = input(f"Enter file number to DELETE (1-{len(files)}) or filename: ").strip()
            target_fname = None
            if sel.isdigit() and 1 <= int(sel) <= len(files):
                target_fname = files[int(sel) - 1]["name"]
            elif any(f["name"].upper() == sel.upper() for f in files):
                target_fname = sel.upper()

            if target_fname:
                conf = input(f"Are you sure you want to permanently delete '{target_fname}'? (y/n): ").strip().lower()
                if conf == "y":
                    mgr.delete_file(target_fname)
            else:
                print("[!] Invalid selection.")

        elif choice == "5":
            status = mgr.get_status()
            print(f"\n[Board Status]: {status}\n")

        elif choice == "6":
            mgr.start_recording()

        elif choice == "7":
            mgr.stop_recording()

        elif choice == "0":
            print("Exiting...")
            break
        else:
            print("[!] Invalid option, please enter 0-7.")


def main():
    parser = argparse.ArgumentParser(description="STM32H750 MicroSD Telemetry CSV Downloader")
    parser.add_argument("--port", "-p", default=None, help="Serial port (e.g. /dev/ttyACM0 or COM3)")
    parser.add_argument("--baud", "-b", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--out", "-o", default="./downloads", help="Download output folder (default: ./downloads)")
    parser.add_argument("--list", "-l", action="store_true", help="List all CSV files on SD card and exit")
    parser.add_argument("--get", "-g", help="Download specific file by name (e.g. --get LOG0001.CSV)")
    parser.add_argument("--all", "-a", action="store_true", help="Download ALL CSV files on SD card")
    parser.add_argument("--start", action="store_true", help="Trigger STM32 to start 10 Hz recording session")
    parser.add_argument("--stop", action="store_true", help="Trigger STM32 to stop recording session and commit file")
    args = parser.parse_args()

    port = args.port or find_default_port()
    if not port:
        print("[ERROR] No USB serial port detected. Connect STM32 USB cable and try again.")
        sys.exit(1)

    mgr = STM32SDManager(port, baudrate=args.baud)
    if not mgr.connect():
        sys.exit(1)

    try:
        if args.start:
            mgr.start_recording()
        elif args.stop:
            mgr.stop_recording()
        elif args.list:
            files = mgr.list_files()
            print_files_table(files)
        elif args.get:
            mgr.download_file(args.get.upper(), args.out)
        elif args.all:
            files = mgr.list_files()
            print_files_table(files)
            if files:
                for f in files:
                    mgr.download_file(f["name"], args.out)
        else:
            interactive_menu(mgr, args.out)
    finally:
        mgr.close()


if __name__ == "__main__":
    main()

