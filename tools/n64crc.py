#!/usr/bin/env python3
"""n64crc.py -- compute, check or rewrite the header checksum of a big-endian (.z64) N64 ROM booted by a CIC-6102.

The boot code (IPL3) checks two words over the first megabyte after the boot code (ROM 0x1000-0x101000) against the
header's words at 0x10 and 0x14. This is the published 6102 algorithm: six 32-bit accumulators start at the chip's
seed 0xF8CA4DDC, and each big-endian data word d updates them in turn:

    t4 += carry of (t6 + d);   t6 += d;   t3 ^= d;   r = d rotated left by (d & 31);   t5 += r;
    t2 ^= r if t2 > d (unsigned) else t6 ^ d;   t1 += t5 ^ d

The checksum is (t6 ^ t4 ^ t3, t5 ^ t2 ^ t1).

usage: tools/n64crc.py ROM                      print the computed and the stored checksum; exit 1 if they differ
       tools/n64crc.py --fix [--pad SIZE] ROM   write the computed checksum into the header, after filling the
                                               ROM with 0xFF to SIZE bytes (or the next power of two above it)
"""

import argparse
import struct
import sys

SEED_6102 = 0xF8CA4DDC
START, LENGTH = 0x1000, 0x100000
HEADER_CRC = 0x10
MASK = 0xFFFFFFFF


def checksum(rom):
    """the CIC-6102 checksum (crc1, crc2) of ROM bytes rom"""
    if len(rom) < START + LENGTH:
        raise ValueError("ROM is 0x%X bytes, shorter than the checksummed 0x%X" % (len(rom), START + LENGTH))
    t1 = t2 = t3 = t4 = t5 = t6 = SEED_6102
    for (d,) in struct.iter_unpack(">I", rom[START:START + LENGTH]):
        s = t6 + d
        if s > MASK:
            t4 = (t4 + 1) & MASK
        t6 = s & MASK
        t3 ^= d
        n = d & 31
        r = ((d << n) | (d >> (32 - n))) & MASK if n else d
        t5 = (t5 + r) & MASK
        t2 ^= r if t2 > d else t6 ^ d
        t1 = (t1 + (t5 ^ d)) & MASK
    return (t6 ^ t4 ^ t3), (t5 ^ t2 ^ t1)


def stored(rom):
    return struct.unpack_from(">II", rom, HEADER_CRC)


def fix(rom):
    """rom with its header checksum recomputed (a new bytes object)"""
    out = bytearray(rom)
    struct.pack_into(">II", out, HEADER_CRC, *checksum(rom))
    return bytes(out)


def pad(rom, size):
    """rom filled with 0xFF (erased flash, as retail pads) to size bytes, or to the next power of two if longer"""
    while size < len(rom):
        size *= 2
    return bytes(rom) + b"\xFF" * (size - len(rom))


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("rom")
    ap.add_argument("--fix", action="store_true", help="write the computed checksum into the header")
    ap.add_argument("--pad", type=lambda s: int(s, 0), help="with --fix: fill with 0xFF to this size first")
    args = ap.parse_args()
    rom = open(args.rom, "rb").read()
    if args.pad and not args.fix:
        sys.exit("n64crc: --pad needs --fix")
    if rom[:4] != b"\x80\x37\x12\x40":
        sys.exit("n64crc: %s is not a big-endian (.z64) ROM" % args.rom)
    if args.fix:
        if args.pad:
            rom = pad(rom, args.pad)
        out = fix(rom)
        open(args.rom, "wb").write(out)
        print("n64crc: %s: 0x%X bytes, checksum %08X %08X" % ((args.rom, len(out)) + stored(out)))
        return 0
    want, have = checksum(rom), stored(rom)
    print("n64crc: %s: computed %08X %08X, header %08X %08X" % (args.rom, want[0], want[1], have[0], have[1]))
    return 0 if want == have else 1


if __name__ == "__main__":
    sys.exit(main())
