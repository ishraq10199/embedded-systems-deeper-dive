#!/usr/bin/env python3
"""
Host-side firmware update script.
Implements the UART update protocol expected by the STM32 bootloader.
In the first 5 seconds of the STM32's boot sequence, run this script.
Example firmware given (example.bin) is a simple blink program.

Usage:
    python3 flash.py <port> <example.bin> [--baud BAUD] [--version VERSION]

Example:
    python3 flash.py /dev/ttyUSB0 example.bin --version 2
"""

import argparse
import struct
import sys

import serial

# Protocol constants — must match firmware.h
FW_UPDATE_REQ  = 0x11
FW_UPDATE_ACK  = 0xAA
FW_UPDATE_SYNC = 0x55
FW_UPDATE_RSND = 0x22
FW_UPDATE_FIN  = 0xFF
FW_UPDATE_ERR  = 0x66

WORDS_PER_CHUNK = 512
BYTES_PER_CHUNK = WORDS_PER_CHUNK * 4  # 2048 bytes
MAX_CHUNK_RETRIES = 10


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

POLYNOMIAL = 0x04C11DB7  # Must match POLYNOMIAL in crc32.h


def crc32_block(crc: int, word: int) -> int:
    """Matches compute_crc32_block() on the STM32."""
    crc ^= word
    for _ in range(32):
        if crc & 0x80000000:
            crc = ((crc << 1) ^ POLYNOMIAL) & 0xFFFFFFFF
        else:
            crc = (crc << 1) & 0xFFFFFFFF
    return crc


def crc32(data: bytes) -> int:
    """
    Matches compute_crc32() on the STM32.
    Data must be word-aligned (length a multiple of 4).
    Words are interpreted as little-endian uint32_t, matching the STM32 memory layout.
    """
    assert len(data) % 4 == 0, "crc32: data length must be a multiple of 4"
    crc = 0xFFFFFFFF
    for i in range(0, len(data), 4):
        word = struct.unpack_from('<I', data, i)[0]
        crc = crc32_block(crc, word)
    return crc


def pack_word(value: int) -> bytes:
    return struct.pack('<I', value)


def read_byte(ser: serial.Serial) -> int:
    data = ser.read(1)
    if not data:
        raise TimeoutError("Timed out waiting for a response from the bootloader")
    return data[0]

def read_byte_until(ser: serial.Serial, timeout: int) -> int:
    """
    Similar to read_byte, but with a specified timeout.
    Will not raise any errors on timeout, but will send a 0xFF byte instead.
    The timeout here will not clash with the one specified in the serial constructor.
    """
    original_timeout = ser.timeout
    ser.timeout = timeout

    data = ser.read(1)
    if not data:
        ser.timeout = original_timeout
        return 0xFF # on timeout
    ser.timeout = original_timeout

    return data[0]

def expect_ack(ser: serial.Serial) -> None:
    byte = read_byte(ser)
    if byte != FW_UPDATE_ACK:
        read_error_lines(ser)
        raise RuntimeError(
            f"Expected ACK (0x{FW_UPDATE_ACK:02X}), got 0x{byte:02X}"
        )


def send_word_with_crc(ser: serial.Serial, value: int) -> None:
    """Send a single word followed by its CRC, as the bootloader expects."""
    word_bytes = pack_word(value)
    ser.write(word_bytes + pack_word(crc32(word_bytes)))


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

def read_error_lines(ser: serial.Serial):
    """
    Read error lines sent from the client when the update process fails
    Keeps reading until a FIN (0xFF) is sent
    """
    print("---Message recieved---")
    received_byte = read_byte(ser)
    while (received_byte != FW_UPDATE_FIN):
        print(received_byte.to_bytes().decode(encoding='ascii'), end='')
        received_byte = read_byte(ser)
    print("---End of Message---")



# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(description='STM32 UART firmware updater')
    parser.add_argument('port',      help='Serial port (e.g. /dev/ttyUSB0 or COM3)')
    parser.add_argument('firmware',  help='Path to firmware binary (.bin)')
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

        # --- Check if device is responds to a firmware update notice (SYNC) ---
        start_ok = True
        ser.write(bytes([FW_UPDATE_SYNC]))
        byte = read_byte_until(ser, 5) # Wait 5 seconds for the device to ACK
        if byte != FW_UPDATE_ACK:
            start_ok = False
            print("UART update request not acknowledged, fallback to standard read")
            while True:
                print(ser.readline().decode(encoding='ascii'), end='')

        if not start_ok:
            # We should never come down here, but if we do, we raise an error
            raise RuntimeError(
                "Aborting standard read, update never started!"
            )

        # If the device has sent an ACK to the update notice, we proceed

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

        # --- Metadata ---
        print("Sending metadata...")
        send_word_with_crc(ser, image_version)
        print("Expecting ACK for version...")
        expect_ack(ser)

        send_word_with_crc(ser, image_size)
        print("Expecting ACK for size...")
        expect_ack(ser)

        send_word_with_crc(ser, image_crc)
        print("Expecting ACK for crc...")
        expect_ack(ser)

        print("Metadata sent!")

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

        # --- Done ---
        ser.write(bytes([FW_UPDATE_FIN]))
        print("\nFirmware transfer complete.")
        print("Switching to standard read from device...")
        print("-----------------------------------------\n")

        while True:
            print(ser.readline().decode(encoding='ascii'), end='')

# Used to debug:
# python uart_firmware_update.py /dev/ttyUSB0 ./example.bin --timeout 10
if __name__ == '__main__':
    try:
        main()
    except (RuntimeError, TimeoutError) as e:
        print(f"\nError: {e}", file=sys.stderr)
        sys.exit(1)