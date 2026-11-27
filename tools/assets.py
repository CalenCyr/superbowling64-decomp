#!/usr/bin/env python3
"""assets.py -- extract the ROM's asset files into editable files, and pack them back byte for byte.

    python3 tools/assets.py extract ROM [-d assets_ext]   write the tree and its manifest.json
    python3 tools/assets.py pack [-d assets_ext] [-o DIR]  rebuild every FAT file into DIR (default <dir>/packed)
    python3 tools/assets.py check ROM [-d assets_ext]      pack in memory; compare each file with the ROM's

The formats come from the game's own loaders:
  * BW64FAT (ROM 0x3C2A0, src/3AD0.c, include/fat.h): an 8-byte magic, then 16-byte entries {name pointer,
    ROM start, ROM end, 0}, ended by a null name pointer. The pointers are RAM addresses (the table loads at
    0x8010A080); the 11-character names follow the entries.
  * The str segment (src/4DC0.c, include/archive.h) is the FAT file "STR     SYS"; its table is the FAT file
    "IDX     SYS": three big-endian words per member (offset from the segment start, stored size, decompressed
    size). Sizes that differ mean the member is LZSS-compressed (4 KB window, flag bytes with bit 1 = literal,
    a match is two bytes: position low 8 bits, then position high 4 bits and length - 3). Members start on even
    offsets; the odd byte between members is kept in the manifest.
  * Images (src/7250.c, Draw_Image and Draw_CiImage): a 16-byte header {type, width, height, 0}, then the palette
    (16 or 256 RGBA5551 entries) and the pixels, stored as two column strips side by side (see _strips).

Extraction writes an image as a PNG only when the PNG converts back to the same bytes; other members stay raw.
Compressed members are written decompressed; pack compresses them again, and the encoder reproduces every
retail stream, so no compressed bytes are kept (a member it could not reproduce would be kept as NNNN.lz).
The ROM is never written, and assets_ext/ is ignored by git.
"""

import argparse
import hashlib
import json
import os
import struct
import sys
import zlib

ROM_SHA1 = "56937a9e9536af55ebed06480413265848c4a04a"
FAT_ROM = 0x3C2A0
FAT_RAM = 0x8010A080
FAT_END = 0x3C470           # the table's file, up to KNSTATICSYS
PAD_START = 0x656CD0        # after the last FAT file; the ROM's fill
PAD_NAME = "PAD"
STR_NAME = "STR     SYS"
IDX_NAME = "IDX     SYS"
IDX_WORDS = 3

LZ_N = 4096
LZ_F = 18
LZ_THRESHOLD = 2


# ===== LZSS =====

def lz_decode(src):
    """The decoder of LZ_Decompress (src/4DC0.c), over the whole of src."""
    window = bytearray(LZ_N)
    r = LZ_N - LZ_F
    out = bytearray()
    pos = 0
    flags = 0
    while pos < len(src):
        flags >>= 1
        if not flags & 0x100:
            flags = src[pos] | 0xFF00
            pos += 1
        if flags & 1:
            c = src[pos]
            pos += 1
            out.append(c)
            window[r] = c
            r = (r + 1) & (LZ_N - 1)
        else:
            i = src[pos]
            j = src[pos + 1]
            pos += 2
            i |= (j & 0xF0) << 4
            for k in range(( j & 0x0F ) + LZ_THRESHOLD + 1):
                c = window[(i + k) & (LZ_N - 1)]
                out.append(c)
                window[r] = c
                r = (r + 1) & (LZ_N - 1)
    return bytes(out)


class _Matcher:
    """The sliding window's string index: one binary search tree per first byte, ordered by the next
    LZ_F - 1 bytes of each window position. The ROM's streams were made by this classic arrangement, whose
    choice among equally long matches depends on the tree's shape; a flat 'nearest' or 'farthest' search
    does not reproduce them."""

    NIL = LZ_N

    def __init__(self):
        n = LZ_N
        self.text = bytearray(n + LZ_F - 1)     # the window, with its first LZ_F - 1 bytes repeated at the end
        self.left = [self.NIL] * (n + 1)
        self.right = [self.NIL] * (n + 257)     # slots n + 1 + byte are the trees' roots
        self.up = [self.NIL] * (n + 1)
        self.length = 0
        self.position = 0

    def insert(self, r):
        """Adds the string at r to its tree and sets length/position to the best match met on the way down;
        an identical string already in the tree is replaced by the newer one."""
        text, left, right, up, nil = self.text, self.left, self.right, self.up, self.NIL
        key = text[r:r + LZ_F]
        cmp_ = 1
        p = LZ_N + 1 + key[0]
        right[r] = left[r] = nil
        best = 0
        while True:
            if cmp_ >= 0:
                if right[p] != nil:
                    p = right[p]
                else:
                    right[p] = r
                    up[r] = p
                    self.length = best
                    return
            else:
                if left[p] != nil:
                    p = left[p]
                else:
                    left[p] = r
                    up[r] = p
                    self.length = best
                    return
            i = 1
            while i < LZ_F:
                cmp_ = key[i] - text[p + i]
                if cmp_:
                    break
                i += 1
            if i > best:
                self.position = p
                best = i
                if best >= LZ_F:
                    break
        up[r] = up[p]
        left[r] = left[p]
        right[r] = right[p]
        up[left[p]] = r
        up[right[p]] = r
        if right[up[p]] == p:
            right[up[p]] = r
        else:
            left[up[p]] = r
        up[p] = nil
        self.length = best

    def delete(self, p):
        """Removes the string at p from its tree (the in-order predecessor takes its place)."""
        left, right, up, nil = self.left, self.right, self.up, self.NIL
        if up[p] == nil:
            return
        if right[p] == nil:
            q = left[p]
        elif left[p] == nil:
            q = right[p]
        else:
            q = left[p]
            if right[q] != nil:
                while right[q] != nil:
                    q = right[q]
                right[up[q]] = left[q]
                up[left[q]] = up[q]
                left[q] = left[p]
                up[left[p]] = q
            right[q] = right[p]
            up[right[p]] = q
        up[q] = up[p]
        if right[up[p]] == p:
            right[up[p]] = q
        else:
            left[up[p]] = q
        up[p] = nil


def lz_encode(data):
    """LZSS-compresses data into a stream lz_decode (and the game) reads back to data."""
    n = len(data)
    m = _Matcher()
    out = bytearray()
    group = bytearray()
    flags = 0
    bit = 0
    s = 0                       # the oldest window position, the next to be replaced
    r = LZ_N - LZ_F             # the look-ahead's first position
    ahead = min(LZ_F, n)
    m.text[r:r + ahead] = data[:ahead]
    pos = ahead
    for i in range(1, LZ_F + 1):
        m.insert(r - i)
    m.insert(r)
    while ahead > 0:
        length = min(m.length, ahead)
        if length <= LZ_THRESHOLD:
            length = 1
            flags |= 1 << bit
            group.append(m.text[r])
        else:
            group.append(m.position & 0xFF)
            group.append(((m.position >> 4) & 0xF0) | (length - LZ_THRESHOLD - 1))
        bit += 1
        if bit == 8:
            out.append(flags)
            out += group
            flags = bit = 0
            group = bytearray()
        i = 0
        while i < length and pos < n:
            c = data[pos]
            pos += 1
            m.delete(s)
            m.text[s] = c
            if s < LZ_F - 1:
                m.text[s + LZ_N] = c
            s = (s + 1) & (LZ_N - 1)
            r = (r + 1) & (LZ_N - 1)
            m.insert(r)
            i += 1
        while i < length:
            i += 1
            m.delete(s)
            s = (s + 1) & (LZ_N - 1)
            r = (r + 1) & (LZ_N - 1)
            ahead -= 1
            if ahead:
                m.insert(r)
    if bit:
        out.append(flags)
        out += group
    return bytes(out)


# ===== PNG (indexed colour, written and read with the standard library) =====

def _chunk(tag, body):
    return (struct.pack(">I", len(body)) + tag + body
            + struct.pack(">I", zlib.crc32(tag + body) & 0xFFFFFFFF))


def png_write(width, height, depth, rows, palette):
    """rows: bytes of height rows, each packed at depth bits per pixel; palette: list of (r, g, b, a)."""
    raw = bytearray()
    stride = len(rows) // height
    for y in range(height):
        raw.append(0)
        raw += rows[y * stride:(y + 1) * stride]
    body = (b"\x89PNG\r\n\x1a\n"
            + _chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, depth, 3, 0, 0, 0))
            + _chunk(b"PLTE", b"".join(bytes(c[:3]) for c in palette))
            + _chunk(b"tRNS", bytes(c[3] for c in palette))
            + _chunk(b"IDAT", zlib.compress(bytes(raw), 9))
            + _chunk(b"IEND", b""))
    return body


def png_read(data):
    """Returns (width, height, depth, rows, palette) of an indexed PNG, rows packed as png_write's are."""
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError("not a PNG")
    pos = 8
    idat = b""
    plte = trns = b""
    while pos < len(data):
        size, tag = struct.unpack(">I4s", data[pos:pos + 8])
        body = data[pos + 8:pos + 8 + size]
        pos += 12 + size
        if tag == b"IHDR":
            width, height, depth, ctype, _, _, interlace = struct.unpack(">IIBBBBB", body)
            if ctype != 3 or interlace or depth not in (1, 2, 4, 8):
                raise ValueError("only non-interlaced indexed PNGs are supported")
        elif tag == b"PLTE":
            plte = body
        elif tag == b"tRNS":
            trns = body
        elif tag == b"IDAT":
            idat += body
    palette = []
    for i in range(len(plte) // 3):
        palette.append((plte[3 * i], plte[3 * i + 1], plte[3 * i + 2], trns[i] if i < len(trns) else 255))
    stride = (width * depth + 7) // 8
    raw = zlib.decompress(idat)
    rows = bytearray()
    prev = bytearray(stride)
    for y in range(height):
        ft = raw[y * (stride + 1)]
        line = bytearray(raw[y * (stride + 1) + 1:(y + 1) * (stride + 1)])
        for x in range(stride):
            a = line[x - 1] if x else 0
            b = prev[x]
            c = prev[x - 1] if x else 0
            if ft == 1:
                line[x] = (line[x] + a) & 255
            elif ft == 2:
                line[x] = (line[x] + b) & 255
            elif ft == 3:
                line[x] = (line[x] + ((a + b) >> 1)) & 255
            elif ft == 4:
                p = a + b - c
                pa, pb, pc = abs(p - a), abs(p - b), abs(p - c)
                pred = a if pa <= pb and pa <= pc else (b if pb <= pc else c)
                line[x] = (line[x] + pred) & 255
        rows += line
        prev = line
    return width, height, depth, bytes(rows), palette


# ===== The game's image file (src/7250.c Draw_CiImage) =====

IMAGE_CI4 = 0
IMAGE_CI8 = 1


def _strips(width, itype):
    """Widths of the two column strips Draw_CiImage draws an image of this width in: the widest power of
    two that fits the padded width (width + width % 16 for CI4, % 8 for CI8), and the power of two covering
    the rest (0 when none is left)."""
    padded = width + (width % 16 if itype == IMAGE_CI4 else width % 8)
    first = 1
    while first * 2 <= padded and first < 512:
        first *= 2
    if padded == first:
        return first, 0
    rest = 2
    while rest < padded - first and rest < 512:
        rest *= 2
    return first, rest


def _rgba5551(v):
    ex = lambda c: (c << 3) | (c >> 2)
    return (ex((v >> 11) & 31), ex((v >> 6) & 31), ex((v >> 1) & 31), 255 if v & 1 else 0)


def _from_rgba(c):
    return ((c[0] >> 3) << 11) | ((c[1] >> 3) << 6) | ((c[2] >> 3) << 1) | (1 if c[3] >= 128 else 0)


def image_to_png(data):
    """The PNG of an image file, or None if data is not one (checked by size) or would not convert back."""
    if len(data) < 16:
        return None
    itype, width, height, unk = struct.unpack(">4I", data[:16])
    if itype not in (IMAGE_CI4, IMAGE_CI8) or unk != 0 or not 0 < width < 1024 or not 0 < height < 1024:
        return None
    first, rest = _strips(width, itype)
    canvas = first + rest
    colours = 16 if itype == IMAGE_CI4 else 256
    depth = 4 if itype == IMAGE_CI4 else 8
    if canvas * depth % 8:
        return None
    pbytes = colours * 2
    ra = first * depth // 8
    rb = rest * depth // 8
    if len(data) != 16 + pbytes + height * (ra + rb):
        return None
    pal = struct.unpack(">%dH" % colours, data[16:16 + pbytes])
    pix = data[16 + pbytes:]
    a = pix[:height * ra]
    b = pix[height * ra:]
    rows = b"".join(a[y * ra:(y + 1) * ra] + b[y * rb:(y + 1) * rb] for y in range(height))
    png = png_write(canvas, height, depth, rows, [_rgba5551(v) for v in pal])
    return png if png_to_image(png, data[:16]) == data else None


def png_to_image(png, header):
    """The image file for a PNG; header is the original file's first 16 bytes (type, width, height)."""
    itype, width, height, unk = struct.unpack(">4I", header)
    first, rest = _strips(width, itype)
    colours = 16 if itype == IMAGE_CI4 else 256
    depth = 4 if itype == IMAGE_CI4 else 8
    pw, ph, pdepth, rows, palette = png_read(png)
    if pw != first + rest or ph != height:
        raise ValueError("image is %dx%d, the file needs %dx%d" % (pw, ph, first + rest, height))
    if pdepth != depth:      # an editor may widen the pixels; the values must still fit
        pixels = _unpack(rows, pw, ph, pdepth)
        if max(pixels) >= colours:
            raise ValueError("pixel value outside the %d-colour palette" % colours)
        rows = _pack(pixels, pw, ph, depth)
    palette = (palette + [(0, 0, 0, 0)] * colours)[:colours]
    stride = pw * depth // 8
    ra = first * depth // 8
    a = b"".join(rows[y * stride:y * stride + ra] for y in range(ph))
    b = b"".join(rows[y * stride + ra:(y + 1) * stride] for y in range(ph))
    return (header + b"".join(struct.pack(">H", _from_rgba(c)) for c in palette) + a + b)


def _unpack(rows, width, height, depth):
    stride = (width * depth + 7) // 8
    out = []
    for y in range(height):
        for x in range(width):
            bit = x * depth
            out.append((rows[y * stride + bit // 8] >> (8 - depth - bit % 8)) & ((1 << depth) - 1))
    return out


def _pack(pixels, width, height, depth):
    stride = (width * depth + 7) // 8
    out = bytearray(stride * height)
    for y in range(height):
        for x in range(width):
            bit = x * depth
            out[y * stride + bit // 8] |= pixels[y * width + x] << (8 - depth - bit % 8)
    return bytes(out)


# ===== ROM structure =====

def read_fat(rom):
    """The BW64FAT entries: [(name, start, end)]."""
    if rom[FAT_ROM:FAT_ROM + 7] != b"BW64FAT":
        raise ValueError("no BW64FAT at ROM 0x%X" % FAT_ROM)
    entries = []
    pos = FAT_ROM + 8
    while True:
        ptr, start, end, _ = struct.unpack(">4I", rom[pos:pos + 16])
        if ptr == 0:
            return entries
        at = FAT_ROM + ptr - FAT_RAM
        entries.append((rom[at:at + 11].decode("ascii"), start, end))
        pos += 16


def _fname(name):
    return name.rstrip()


def split_idx(idx):
    """(entries, tail) of the IDX file: the monotonic run of 12-byte entries, then whatever remains."""
    entries = []
    prev_end = 0
    while (len(entries) + 1) * 12 <= len(idx):
        off, size, raw = struct.unpack(">3I", idx[len(entries) * 12:len(entries) * 12 + 12])
        if off - prev_end not in (0, 1) or size == 0 or (size != raw and raw < size // 2):
            break
        entries.append((off, size, raw))
        prev_end = off + size
    return entries, idx[len(entries) * 12:]


def extract(rom, out_dir):
    fat = read_fat(rom)
    manifest = {"rom_sha1": hashlib.sha1(rom).hexdigest(), "fat_table": {"rom": FAT_ROM, "end": FAT_END},
                "files": [], "str": None, "pad": {"rom": PAD_START, "end": len(rom)}}
    os.makedirs(os.path.join(out_dir, "files"), exist_ok=True)

    def put(path, data):
        full = os.path.join(out_dir, path)
        os.makedirs(os.path.dirname(full), exist_ok=True)
        with open(full, "wb") as f:
            f.write(data)

    put("files/BW64FAT.bin", rom[FAT_ROM:FAT_END])
    put("files/%s.bin" % PAD_NAME, rom[PAD_START:])
    byname = {}
    for name, start, end in fat:
        byname[name] = (start, end)
        manifest["files"].append({"name": name, "rom": start, "end": end,
                                  "file": None if name == STR_NAME else "files/%s.bin" % _fname(name)})
        if name != STR_NAME:
            put("files/%s.bin" % _fname(name), rom[start:end])
    # the str segment: its members, in IDX order
    sstart, send = byname[STR_NAME]
    istart, iend = byname[IDX_NAME]
    entries, tail_idx = split_idx(rom[istart:iend])
    members = []
    pos = 0
    for n, (off, size, raw) in enumerate(entries):
        stored = rom[sstart + off:sstart + off + size]
        nxt = entries[n + 1][0] if n + 1 < len(entries) else send - sstart
        gap = rom[sstart + off + size:sstart + nxt]
        m = {"index": n, "gap": gap.hex()}
        if size == raw:
            data = stored
            m["compressed"] = False
        else:
            data = lz_decode(stored)
            if len(data) != raw:
                raise ValueError("member %d decompresses to %d bytes, not %d" % (n, len(data), raw))
            m["compressed"] = True
            if lz_encode(data) != stored:
                m["lz"] = "str/%04d.lz" % n         # the encoder does not reproduce it: keep its bytes
                put(m["lz"], stored)
        png = image_to_png(data)
        if png is not None:
            m["file"] = "str/%04d.png" % n
            put(m["file"], png)
            m["image_header"] = data[:16].hex()
        else:
            m["file"] = "str/%04d.bin" % n
            put(m["file"], data)
        members.append(m)
        pos = nxt
    manifest["str"] = {"members": members, "idx_tail": tail_idx.hex(), "first_offset": entries[0][0]}
    with open(os.path.join(out_dir, "manifest.json"), "w") as f:
        json.dump(manifest, f, indent=1)
        f.write("\n")
    return manifest


def _member_data(m, base):
    with open(os.path.join(base, m["file"]), "rb") as f:
        data = f.read()
    if m["file"].endswith(".png"):
        data = png_to_image(data, bytes.fromhex(m["image_header"]))
    return data


def pack_files(base):
    """{FAT name: bytes} for every FAT file, plus "BW64FAT" and "PAD", rebuilt from the tree at base."""
    with open(os.path.join(base, "manifest.json")) as f:
        manifest = json.load(f)
    out = {}
    for ent in manifest["files"]:
        if ent["file"]:
            with open(os.path.join(base, ent["file"]), "rb") as f:
                out[ent["name"]] = f.read()
    for key, fname in (("BW64FAT", "files/BW64FAT.bin"), (PAD_NAME, "files/%s.bin" % PAD_NAME)):
        with open(os.path.join(base, fname), "rb") as f:
            out[key] = f.read()
    seg = bytearray()
    idx = bytearray()
    members = manifest["str"]["members"]
    for n, m in enumerate(members):
        data = _member_data(m, base)
        if m["compressed"]:
            if "lz" in m:
                with open(os.path.join(base, m["lz"]), "rb") as f:
                    stored = f.read()
            else:
                stored = lz_encode(data)
            if len(stored) > 0xFFFF:
                raise ValueError("member %d compresses to %d bytes; LZ_Decompress takes a 16-bit length"
                                 % (n, len(stored)))
        else:
            stored = data
        if len(seg) % 2:
            raise AssertionError("misaligned member")
        idx += struct.pack(">3I", len(seg), len(stored), len(data))
        seg += stored
        gap = bytes.fromhex(m["gap"])
        if len(seg) % 2 and not gap:
            gap = b"\0"                              # DMA reads whole halfwords, and the next member is even
        seg += gap
    out[STR_NAME] = bytes(seg)
    out[IDX_NAME] = bytes(idx) + bytes.fromhex(manifest["str"]["idx_tail"])
    return manifest, out


def pack(base, dest):
    manifest, files = pack_files(base)
    os.makedirs(dest, exist_ok=True)
    for name, data in files.items():
        with open(os.path.join(dest, _fname(name) + ".bin"), "wb") as f:
            f.write(data)
    return files


def check(rom, base):
    """Compares every packed file with the ROM's bytes; returns the list of mismatches."""
    manifest, files = pack_files(base)
    ranges = {e["name"]: (e["rom"], e["end"]) for e in manifest["files"]}
    ranges["BW64FAT"] = (manifest["fat_table"]["rom"], manifest["fat_table"]["end"])
    ranges[PAD_NAME] = (manifest["pad"]["rom"], manifest["pad"]["end"])
    bad = []
    for name, (start, end) in sorted(ranges.items(), key=lambda kv: kv[1]):
        if files[name] != rom[start:end]:
            bad.append("%s (ROM 0x%X-0x%X)" % (name.strip(), start, end))
    return bad, len(ranges), len(manifest["str"]["members"])


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    sub = ap.add_subparsers(dest="cmd", required=True)
    for name in ("extract", "check"):
        p = sub.add_parser(name)
        p.add_argument("rom")
        p.add_argument("-d", "--dir", default="assets_ext")
    p = sub.add_parser("pack")
    p.add_argument("-d", "--dir", default="assets_ext")
    p.add_argument("-o", "--out")
    args = ap.parse_args(argv)
    if args.cmd in ("extract", "check"):
        with open(args.rom, "rb") as f:
            rom = f.read()
        if hashlib.sha1(rom).hexdigest() != ROM_SHA1:
            print("assets: %s is not the retail ROM (SHA-1 mismatch)" % args.rom, file=sys.stderr)
            return 1
    if args.cmd == "extract":
        m = extract(rom, args.dir)
        imgs = sum(1 for x in m["str"]["members"] if x["file"].endswith(".png"))
        print("assets: extracted %d FAT files and %d str members (%d as PNG) to %s"
              % (len(m["files"]), len(m["str"]["members"]), imgs, args.dir))
    elif args.cmd == "pack":
        files = pack(args.dir, args.out or os.path.join(args.dir, "packed"))
        print("assets: packed %d files" % len(files))
    else:
        bad, nfiles, nmem = check(rom, args.dir)
        if bad:
            print("assets: MISMATCH: " + ", ".join(bad), file=sys.stderr)
            return 1
        print("ASSETS OK: %d files (%d str members) pack to the ROM's bytes" % (nfiles, nmem))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
