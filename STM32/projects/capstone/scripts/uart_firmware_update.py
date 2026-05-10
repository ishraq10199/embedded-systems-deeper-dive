#!/usr/bin/env python3
"""
Host-side firmware update script.
Implements the UART update protocol expected by the STM32 bootloader.

Usage:
    python3 flash.py <port> <firmware.bin> [--baud BAUD] [--version VERSION]

Example:
    python3 flash.py /dev/ttyUSB0 firmware.bin --version 2
"""

import argparse
import struct
import sys
import time
import zlib

import serial

# Protocol constants — must match firmware.h
FW_UPDATE_REQ  = 0x11
FW_UPDATE_ACK  = 0xAA
FW_UPDATE_SYNC = 0x55
FW_UPDATE_RSND = 0x22
FW_UPDATE_FIN  = 0xFF

WORDS_PER_CHUNK = 512
BYTES_PER_CHUNK = WORDS_PER_CHUNK * 4  # 2048 bytes
MAX_CHUNK_RETRIES = 10


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def crc32(data: bytes) -> int:
    """Must match compute_crc32() in the bootloader."""
    return zlib.crc32(data) & 0xFFFFFFFF


def pack_word(value: int) -> bytes:
    return bytes(list((value).to_bytes(4, byteorder='little')))
    # return struct.pack('<I', value)


def read_byte(ser: serial.Serial) -> int:
    data = ser.read(1)
    if not data:
        raise TimeoutError("Timed out waiting for a response from the bootloader")
    return data[0]


def expect_ack(ser: serial.Serial) -> None:
    print("Expecting ACK")
    byte = read_byte(ser)
    if byte != FW_UPDATE_ACK:
        raise RuntimeError(
            f"Expected ACK (0x{FW_UPDATE_ACK:02X}), got 0x{byte:02X}"
        )


def send_word_with_crc(ser: serial.Serial, value: int) -> None:
    """Send a single word followed by its CRC, as the bootloader expects."""
    word_bytes = pack_word(value)
    msg = word_bytes + pack_word(crc32(word_bytes))
    print(list(msg))
    ser.write(word_bytes)
    time.sleep(0.1)
    ser.write(pack_word(crc32(word_bytes)))


def load_firmware(path: str):
    """
    Read the firmware binary and return:
      - original_size : byte count before any padding (sent as image_size)
      - fw_data       : firmware padded to a full chunk boundary for transmission
      - image_crc     : CRC32 over the word-aligned firmware (matches the
                        bootloader's final compute_crc32(appRomStart, image_words))
    """
    with open(path, 'rb') as f:
        raw = f.read()

    original_size = len(raw)

    # Pad to word boundary with 0xFF (erased flash state)
    word_pad = (4 - original_size % 4) % 4
    fw_word_aligned = raw + b'\xFF' * word_pad

    # CRC is over the word-aligned data only — matches image_words on the target
    image_crc = crc32(fw_word_aligned)

    # Pad further to a full chunk boundary for transmission
    chunk_pad = (BYTES_PER_CHUNK - len(fw_word_aligned) % BYTES_PER_CHUNK) % BYTES_PER_CHUNK
    fw_data = fw_word_aligned + b'\xFF' * chunk_pad

    return original_size, fw_data, image_crc


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(description='STM32 UART firmware updater')
    parser.add_argument('port',     help='Serial port (e.g. /dev/ttyUSB0 or COM3)')
    parser.add_argument('firmware', help='Path to firmware binary (.bin)')
    parser.add_argument('--baud',    type=int,   default=115200)
    parser.add_argument('--version', type=int,   default=1,   help='Firmware version (uint32)')
    parser.add_argument('--timeout', type=float, default=5.0, help='Read timeout in seconds')
    args = parser.parse_args()

    image_size, fw_data, image_crc = load_firmware(args.firmware)
    image_version = args.version
    num_chunks = len(fw_data) // BYTES_PER_CHUNK

    print(f"Firmware : {args.firmware}")
    print(f"  Size   : {image_size} bytes ({num_chunks} chunk(s))")
    print(f"  Version: 0x{image_version:08X}")
    print(f"  CRC    : 0x{image_crc:08X}")

    with serial.Serial(args.port, args.baud, timeout=args.timeout) as ser:

        # --- Handshake ---
        print("\nWaiting for bootloader REQ...")
        byte = read_byte(ser)
        if byte != FW_UPDATE_REQ:
            raise RuntimeError(
                f"Expected REQ (0x{FW_UPDATE_REQ:02X}), got 0x{byte:02X}"
            )

        ser.write(bytes([FW_UPDATE_ACK, FW_UPDATE_SYNC]))
        expect_ack(ser)
        print("Handshake OK")

        time.sleep(0.1)

        # --- Metadata ---
        print("Sending metadata...")
        send_word_with_crc(ser, image_version)
        expect_ack(ser)

        time.sleep(0.1)

        send_word_with_crc(ser, image_size)
        expect_ack(ser)

        send_word_with_crc(ser, image_crc)
        expect_ack(ser)

        # --- Chunk transfer ---
        print(f"Transferring {num_chunks} chunk(s)...")
        for i in range(num_chunks):
            chunk = fw_data[i * BYTES_PER_CHUNK : (i + 1) * BYTES_PER_CHUNK]
            chunk_crc = crc32(chunk)
            retries = 0

            while True:
                ser.write(chunk + pack_word(chunk_crc))
                response = read_byte(ser)

                if response == FW_UPDATE_ACK:
                    print(f"  Chunk {i + 1}/{num_chunks} OK")
                    break
                elif response == FW_UPDATE_RSND:
                    retries += 1
                    print(f"  Chunk {i + 1}/{num_chunks} — CRC error, retry {retries}/{MAX_CHUNK_RETRIES}")
                    if retries >= MAX_CHUNK_RETRIES:
                        raise RuntimeError("Too many chunk retries — aborting")
                else:
                    raise RuntimeError(f"Unexpected response 0x{response:02X}")
                time.sleep(0.1)

        # --- Done ---
        ser.write(bytes([FW_UPDATE_FIN]))
        print("\nFirmware transfer complete.")


if __name__ == '__main__':
    try:
        main()
    except (RuntimeError, TimeoutError) as e:
        print(f"\nError: {e}", file=sys.stderr)
        sys.exit(1)