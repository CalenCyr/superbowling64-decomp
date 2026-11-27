#!/usr/bin/env python3
"""Tests for tools/n64crc.py: a vector derived by hand, the retail ROM's stored checksum (when rom.z64 is present),
the range the checksum covers, the padding, and the command line's check and --fix."""
import os
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE.parent))
import n64crc  # noqa: E402

ROOT = HERE.parent.parent
RETAIL = ROOT / "rom.z64"
RETAIL_CRC = (0xAA1D215A, 0x91CBBE9A)
SIZE = n64crc.START + n64crc.LENGTH


def blank():
    rom = bytearray(SIZE)
    rom[:4] = b"\x80\x37\x12\x40"
    return rom


def main():
    errors = []
    seed = n64crc.SEED_6102

    # All-zero data: only t1 changes, by t5 (the seed) per word, so crc1 = seed and crc2 = seed * (words + 1).
    want = (seed, seed * (n64crc.LENGTH // 4 + 1) & n64crc.MASK)
    got = n64crc.checksum(bytes(blank()))
    if got != want:
        errors.append("zero data: got %08X %08X, want %08X %08X" % (got + want))

    # Bytes outside 0x1000-0x101000 do not count; a byte inside does.
    rom = blank()
    rom[0x20] = rom[n64crc.START - 1] = 0x55
    rom += b"\xFF" * 16
    if n64crc.checksum(bytes(rom)) != want:
        errors.append("a byte outside the checksummed range changed the checksum")
    rom[0x1234] = 1
    if n64crc.checksum(bytes(rom)) == want:
        errors.append("a byte inside the checksummed range did not change the checksum")

    if RETAIL.exists():
        data = RETAIL.read_bytes()
        if n64crc.checksum(data) != RETAIL_CRC or n64crc.stored(data) != RETAIL_CRC:
            errors.append("retail rom.z64: the computed or stored checksum is not %08X %08X" % RETAIL_CRC)
    else:
        print("n64crc_tests: rom.z64 missing, retail check skipped")

    # The command line: a stale header fails the check; --fix writes only the header's two words; then it passes.
    with tempfile.TemporaryDirectory() as tmp:
        path = os.path.join(tmp, "t.z64")
        open(path, "wb").write(bytes(rom))
        tool = [sys.executable, str(HERE.parent / "n64crc.py")]
        if subprocess.run(tool + [path], capture_output=True).returncode != 1:
            errors.append("check of a stale header did not exit 1")
        subprocess.run(tool + ["--fix", path], capture_output=True, check=True)
        fixed = open(path, "rb").read()
        diff = [i for i in range(len(rom)) if rom[i] != fixed[i]]
        if not diff or min(diff) < 0x10 or max(diff) > 0x17:
            errors.append("--fix changed bytes outside the header checksum: %s" % diff[:4])
        if struct.unpack_from(">II", fixed, 0x10) != n64crc.checksum(fixed):
            errors.append("--fix did not write the computed checksum")
        if subprocess.run(tool + [path], capture_output=True).returncode != 0:
            errors.append("check after --fix did not pass")

    # --pad: 0xFF up to the size, or the next power of two above it.
    if n64crc.pad(b"ab", 8) != b"ab" + b"\xFF" * 6 or len(n64crc.pad(bytes(9), 4)) != 16:
        errors.append("pad: wrong length or fill")

    for e in errors:
        print("n64crc_tests: FAIL: " + e)
    print("n64crc_tests: %s" % ("all passed" if not errors else "%d failure(s)" % len(errors)))
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
