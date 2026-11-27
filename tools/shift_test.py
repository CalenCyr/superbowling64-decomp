#!/usr/bin/env python3
"""shift_test.py -- check that the ROM still links correctly when code or data changes size.

Relinks the objects of a finished build twice into build/shift/ (the normal build and the gate are untouched):

  base     the normal linker script, plus a marker symbol at the insertion point;
  shifted  the same, with SHIFT bytes of padding inserted at the insertion point (by default right after the
           first C unit's .text in main), as if that unit had grown.

Both links keep their relocations (ld -q). The base ROM must equal build/superbowling.z64. The checks, each on the
shifted link compared with the base:

  symbols    every symbol whose base value lies after the insertion point (VRAM from the insertion to the end of
             the home segment's bss and of every segment linked after it in VRAM, such as the fat, knstatic, RDP
             FIFO and overlay segments after main; or a ROM offset from the insertion to the end of the ROM) moves
             by SHIFT, and
             every other symbol stays (a symbol that stays is pinned to its retail address, for example by an
             absolute assignment); sizes (`*_SIZE`, `*_size`) are not checked;
  words      no 32-bit word of the code and data segments (not the binary assets) still holds a moved address (or a moved segment's ROM start or end)
             unchanged: a pointer the link does not relocate, such as a raw number in an asm data blob;
  lui        no `lui` without a relocation loads the upper half of a moved address (a hard-coded pointer in code;
             a `lui` followed by an `ori` of the same register is taken as a constant);
  overlap    the sections that are not overlays do not overlap in VRAM, nor in ROM.

Known blockers are listed in config/shift_known.txt (one `<check> <name>` per line, `#` comments); the test
fails on any finding that is not listed (-v also lists the known ones not found at this insertion point).

usage: tools/shift_test.py [--shift N] [--after 'build/src/1060.o(.text)'] [-v]
"""

import argparse
import bisect
import hashlib
import os
import re
import subprocess
import sys

CROSS = os.environ.get("CROSS", "mips64-linux-gnu-")
LD = [CROSS + "ld", "-m", "elf32btsmip"]
BUILD = "build"
OUT = os.path.join(BUILD, "shift")
LD_SCRIPT = "superbowling.ld"
ELF = os.path.join(BUILD, "superbowling.elf")
ROM = os.path.join(BUILD, "superbowling.z64")
LIBULTRA = os.path.join(BUILD, "ultralib/K/libgultra_rom/libgultra_rom.a")
SCRIPTS = ["undefined_syms.txt", os.path.join(BUILD, "undefined_syms_auto.ld"),
           os.path.join(BUILD, "undefined_funcs_auto.ld")]
KNOWN = "config/shift_known.txt"
MARK = "__shift_point"

# sections that share the overlay slot: they overlap each other by design
OVERLAYS = ("gacode", "rscode", "gscode", "cacode", "pvcode", "mgcode", "opcode")


def run(cmd):
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode != 0:
        sys.exit("shift-test: command failed: %s\n%s%s" % (" ".join(cmd), r.stdout, r.stderr))
    return r.stdout


def link(name, after, pad):
    script = open(LD_SCRIPT).read()
    line = "        %s;\n" % after
    if script.count(line) != 1:
        sys.exit("shift-test: %r is not exactly one line of %s" % (after, LD_SCRIPT))
    insert = line + "        %s = .;\n" % MARK
    if pad:
        insert += "        . += 0x%X;\n" % pad
    path = os.path.join(OUT, name + ".ld")
    open(path, "w").write(script.replace(line, insert))
    elf = os.path.join(OUT, name + ".elf")
    rom = os.path.join(OUT, name + ".z64")
    cmd = LD + ["-q", "-T", path]
    for s in SCRIPTS:
        cmd += ["-T", s]
    run(cmd + ["--no-check-sections", "-Map", os.path.join(OUT, name + ".map"), "-o", elf, LIBULTRA])
    run([CROSS + "objcopy", "-O", "binary", elf, rom])
    return Image(elf, rom)


class Image:
    def __init__(self, elf, rom):
        self.rom = open(rom, "rb").read()
        self.sections = []      # (name, vma, lma, size, flags)
        out = run([CROSS + "objdump", "-h", elf]).splitlines()
        for i, l in enumerate(out):
            m = re.match(r"\s*\d+\s+(\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)\s+([0-9a-f]+)", l)
            if m:
                self.sections.append((m.group(1), int(m.group(3), 16), int(m.group(4), 16), int(m.group(2), 16),
                                      out[i + 1]))
        self.ndx = {"ABS": "ABS"}   # readelf section index -> section name
        for l in run(["readelf", "-SW", elf]).splitlines():
            m = re.match(r"\s*\[\s*(\d+)\]\s+(\S+)", l)
            if m:
                self.ndx[m.group(1)] = m.group(2)
        self.symbols = {}       # name -> sorted list of (value, section name)
        for l in run(["readelf", "-sW", elf]).splitlines():
            f = l.split()
            if len(f) < 8 or not f[0][:-1].isdigit() or f[3] in ("SECTION", "FILE") or f[6] == "UND":
                continue
            self.symbols.setdefault(f[7], []).append((int(f[1], 16), self.ndx.get(f[6], f[6])))
        for v in self.symbols.values():
            v.sort()
        self.relocs = set()     # VRAM addresses holding a relocated field
        for l in run(["readelf", "-rW", elf]).splitlines():
            m = re.match(r"([0-9a-f]{8})\s+[0-9a-f]+\s+R_MIPS_(\w+)", l)
            if m:
                self.relocs.add(int(m.group(1), 16))

    def sym(self, name):
        return self.symbols[name][0][0]

    def section(self, name):
        for s in self.sections:
            if s[0] == name:
                return s
        sys.exit("shift-test: no section " + name)

    def owner(self, vma, section):
        """the nearest named symbol at or below vma within the section, for reports"""
        if not hasattr(self, "_named"):
            self._named = sorted((v, name) for name, vals in self.symbols.items() for v, ndx in vals
                                 if ndx != "ABS" and not name.startswith(".") and not name.endswith("NON_MATCHING")
                                 and not re.search(r"_(VRAM|START|END)$", name))
        i = bisect.bisect_right(self._named, (vma, "\xff")) - 1
        if i < 0 or self._named[i][0] < section[1]:
            return "0x%08X" % vma
        v, name = self._named[i]
        return "%s+0x%X" % (name, vma - v) if vma != v else name


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("--shift", type=lambda s: int(s, 0), default=0x40)
    ap.add_argument("--after", default="build/src/1060.o(.text)")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    if args.shift % 16:
        sys.exit("shift-test: --shift must be a multiple of 16 (objects align to 16)")
    for f in [ELF, ROM, LIBULTRA] + SCRIPTS:
        if not os.path.exists(f):
            sys.exit("shift-test: %s is missing: run make first" % f)
    os.makedirs(OUT, exist_ok=True)

    base = link("base", args.after, 0)
    shifted = link("shifted", args.after, args.shift)
    if hashlib.sha1(base.rom).digest() != hashlib.sha1(open(ROM, "rb").read()).digest():
        sys.exit("shift-test: the unshifted relink differs from %s" % ROM)
    N = args.shift

    # where the insertion is, in VRAM and in ROM
    P = base.sym(MARK)
    home = base.section(base.symbols[MARK][0][1])
    R = home[2] + (P - home[1])
    seg = home[0].lstrip(".")
    rom_end = len(base.rom)
    if len(shifted.rom) != rom_end + N:
        print("note: the ROM grew by 0x%X, not 0x%X (alignment)" % (len(shifted.rom) - rom_end, N))

    overlay = seg in OVERLAYS

    def relevant(section):
        # an overlay shares its VRAM with the others: only its own sections and the fixed ones count
        part = section.lstrip(".").split("_")[0]
        return not overlay or part not in OVERLAYS or part == seg

    # a section that is exactly one binary asset (`ld -b binary`): opaque game files, not scanned; their VRAM
    # is meaningless (they are never loaded at it)
    assets = {(vals[0][0], base.sym(name[:-6] + "_end")) for name, vals in base.symbols.items()
              if name.startswith("_binary_") and name.endswith("_start")}
    asset_secs = {s[0] for s in base.sections if (s[1], s[1] + s[3]) in assets}

    # what moves in VRAM: the rest of the home segment (and its bss), and every section linked after it in VRAM
    # (the segments that follow main's bss: fat, knstatic, the RDP FIFO and the overlay slot), up to the end of the
    # last; for an overlay, the other overlays are not relevant (they share the slot)
    home_secs = {home[0], home[0] + "_bss"}
    moved_secs = home_secs | {s[0] for s in base.sections if s[1] >= 0x80000000 and s[1] > P
                              and s[0] not in asset_secs and relevant(s[0])}
    vram_end = max(base.sym(seg + "_BSS_END") if seg + "_BSS_END" in base.symbols else home[1] + home[3],
                   max(s[1] + s[3] for s in base.sections if s[0] in moved_secs))

    def moved_vram(v):
        return P <= v < vram_end

    findings = []       # (check, name, detail)

    # 1. symbols
    for name, vals in sorted(base.symbols.items()):
        if name in (MARK, "gcc2_compiled.") or name.startswith(".L") or re.search(r"_(SIZE|size)$", name):
            continue
        new = shifted.symbols.get(name)
        if new is None or len(new) != len(vals):
            continue
        # a name defined more than once (statics): pair the definitions section by section, in address order
        pairs = []
        for sec in sorted({n for _, n in vals}):
            pairs += zip([x for x in vals if x[1] == sec], [y for y in new if y[1] == sec])
        prefix = re.match(r"([a-z0-9]+)_(VRAM|VRAM_END|BSS_START|BSS_END|TEXT_|DATA_|RODATA_)", name)
        for (v, ndx), (w, _) in pairs:
            if ndx in asset_secs and v >= 0x80000000:
                continue
            if ndx != "ABS" and not relevant(ndx):
                continue
            if overlay and prefix and prefix.group(1) != seg and not relevant(prefix.group(1)):
                continue    # another overlay's linker bound, placed by ld in whichever section holds its value
            if ndx == "ABS" and overlay and moved_vram(v) and not (name.startswith(seg + "_")
                                                                   or name.endswith("_" + seg)):
                continue
            if ndx in moved_secs or ndx == "ABS":
                moves = moved_vram(v)
            else:
                moves = False
            if moves:
                want = v + N
            elif ndx == "ABS" and R <= v <= rom_end and v < 0x80000000:
                want = v + N
            else:
                want = v
            if w != want and not (v in (P, vram_end) and w in (v, v + N)):
                findings.append(("symbol", name, "0x%08X -> 0x%08X, expected 0x%08X%s" % (
                    v, w, want, " (absolute)" if ndx == "ABS" else "")))

    # 2. words that still hold a moved address
    boundaries = set()
    for name, vals in base.symbols.items():
        if re.search(r"_ROM_(START|END)$", name) and R <= vals[0][0] <= rom_end:
            boundaries.add(vals[0][0])
    for s in base.sections:
        name, vma, lma, size, flags = s
        if "CONTENTS" not in flags or lma >= len(base.rom) or (vma, vma + size) in assets or not relevant(name):
            continue
        for off in range(0, size - 3, 4):
            a = lma + off
            if R <= a < R + N:
                continue
            b = a if a < R else a + N
            w = int.from_bytes(base.rom[a:a + 4], "big")
            w2 = int.from_bytes(shifted.rom[b:b + 4], "big")
            if w != w2:
                continue
            if moved_vram(w) or (w in boundaries and "CODE" not in flags):
                findings.append(("word", base.owner(vma + off, s), "ROM 0x%X holds 0x%08X" % (a, w)))

    # 3. lui without a relocation that loads the upper half of a moved address
    lo_hi, hi_hi = (P + 0x8000) >> 16, (vram_end + 0x8000) >> 16
    for s in shifted.sections:
        name, vma, lma, size, flags = s
        if "CODE" not in flags or "CONTENTS" not in flags or not relevant(name):
            continue
        for off in range(0, size - 3, 4):
            ins = int.from_bytes(shifted.rom[lma + off:lma + off + 4], "big")
            nxt = int.from_bytes(shifted.rom[lma + off + 4:lma + off + 8], "big")
            if nxt >> 26 == 0x0D and (nxt >> 21) & 31 == (ins >> 16) & 31:
                continue    # lui then ori: a mask or other constant, not an address (addresses take addiu)
            if ins >> 26 == 0x0F and lo_hi <= (ins & 0xFFFF) <= hi_hi and vma + off not in shifted.relocs:
                findings.append(("lui", shifted.owner(vma + off, s), "0x%08X: lui 0x%04X, no relocation" % (
                    vma + off, ins & 0xFFFF)))

    # 4. overlaps among the sections that are not overlays
    fixed = [s for s in shifted.sections
             if s[3] and not s[0].lstrip(".").split("_")[0] in OVERLAYS and s[1] >= 0x80000000]
    for i, a in enumerate(fixed):
        for b in fixed[i + 1:]:
            if a[1] < b[1] + b[3] and b[1] < a[1] + a[3]:
                findings.append(("overlap", "%s/%s" % (a[0], b[0]), "VRAM 0x%08X-0x%08X and 0x%08X-0x%08X" % (
                    a[1], a[1] + a[3], b[1], b[1] + b[3])))
    loaded = sorted((s[2], s[2] + s[3], s[0]) for s in shifted.sections if "CONTENTS" in s[4] and s[3])
    for (s1, e1, n1), (s2, e2, n2) in zip(loaded, loaded[1:]):
        if s2 < e1:
            findings.append(("overlap", "%s/%s" % (n1, n2), "ROM 0x%X-0x%X and 0x%X-0x%X" % (s1, e1, s2, e2)))

    known = set()
    if os.path.exists(KNOWN):
        for l in open(KNOWN):
            l = l.split("#")[0].split()
            if len(l) >= 2:
                known.add((l[0], l[1]))

    def key(f):
        # a word or lui finding is known by its owning symbol, without the offset
        return (f[0], f[1].split("+")[0])

    new = [f for f in findings if key(f) not in known]
    seen = {key(f) for f in findings}
    gone = sorted(known - seen)
    counts = {}
    for f in findings:
        counts[f[0]] = counts.get(f[0], 0) + 1
    print("shift-test: +0x%X bytes after %s (VRAM 0x%08X, ROM 0x%X); moved: VRAM 0x%08X-0x%08X, ROM 0x%X-0x%X"
          % (N, args.after, P, R, P, vram_end, R, rom_end))
    print("findings: %s" % (", ".join("%s %d" % kv for kv in sorted(counts.items())) or "none"))
    for f in findings if args.verbose else new:
        print("  %s%s %s: %s" % ("" if key(f) not in known else "(known) ", f[0], f[1], f[2]))
    if args.verbose:
        for g in gone:
            print("  known, not found at this insertion point: %s %s" % g)
    if new:
        print("SHIFT FAIL: %d finding(s) not in %s" % (len(new), KNOWN))
        return 1
    print("SHIFT OK: %d finding(s), all known blockers (%s)" % (len(findings), KNOWN) if findings
          else "SHIFT OK: nothing points at an old address")
    return 0


if __name__ == "__main__":
    sys.exit(main())
