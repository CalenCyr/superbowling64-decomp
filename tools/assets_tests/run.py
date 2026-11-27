#!/usr/bin/env python3
"""Synthetic tests for tools/assets.py: the LZSS encoder against the game's decoder (including zero runs and
data shorter than one look-ahead), the PNG reader on filtered rows, and the image file <-> PNG round trip for
both colour-indexed types, odd widths and an edited palette."""
import random
import struct
import sys
import zlib
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent.parent))
import assets  # noqa: E402


def lzss_cases():
    rnd = random.Random(1)
    yield b""
    yield b"a"
    yield b"ab" * 5
    yield bytes(100)
    yield bytes(5000) + b"xyz" * 2000
    yield bytes(rnd.randrange(4) for _ in range(9000))
    yield bytes(rnd.randrange(256) for _ in range(3000))
    yield (b"0123456789abcdefgh" * 300) + bytes(rnd.randrange(256) for _ in range(40))


def image(itype, width, height, rnd):
    first, rest = assets._strips(width, itype)
    colours = 16 if itype == 0 else 256
    pix = (first + rest) * height // (2 if itype == 0 else 1)
    return (struct.pack(">4I", itype, width, height, 0)
            + b"".join(struct.pack(">H", rnd.randrange(65536)) for _ in range(colours))
            + bytes(rnd.randrange(256) for _ in range(pix)))


def main():
    errors = []
    for n, data in enumerate(lzss_cases()):
        enc = assets.lz_encode(data)
        if assets.lz_decode(enc) != data:
            errors.append("lzss case %d does not round-trip" % n)
    rnd = random.Random(2)
    for itype, width, height in ((0, 33, 7), (0, 64, 4), (1, 72, 12), (1, 100, 5), (0, 16, 3)):
        data = image(itype, width, height, rnd)
        png = assets.image_to_png(data)
        if png is None:
            errors.append("image %d %dx%d not converted" % (itype, width, height))
            continue
        if assets.png_to_image(png, data[:16]) != data:
            errors.append("image %d %dx%d does not round-trip" % (itype, width, height))
        # a PNG re-saved with Paeth-filtered rows reads back the same
        w, h, depth, rows, pal = assets.png_read(png)
        stride = len(rows) // h
        raw = bytearray()
        for y in range(h):
            raw.append(4)
            prev = rows[(y - 1) * stride:y * stride] if y else bytes(stride)
            for x in range(stride):
                a = rows[y * stride + x - 1] if x else 0
                b = prev[x]
                c = prev[x - 1] if x else 0
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                raw.append((rows[y * stride + x] - pred) & 255)
        plain = assets.png_write(w, h, depth, rows, pal)
        refiltered = (plain[:plain.index(b"IDAT") - 4]
                      + assets._chunk(b"IDAT", zlib.compress(bytes(raw))) + assets._chunk(b"IEND", b""))
        if assets.png_to_image(refiltered, data[:16]) != data:
            errors.append("image %d %dx%d: filtered PNG reads differently" % (itype, width, height))
    if errors:
        print("\n".join("FAIL: " + e for e in errors))
        return 1
    print("assets tests OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
