#!/usr/bin/env python3
"""equiv.py -- differential test of a parked NON_MATCHING draft against the retail function.

    tools/equiv.py [FUNC ...] [--cases N] [--seed S] [-j JOBS] [-v]

With no FUNC, tests every function that has a `#ifdef NON_MATCHING` draft in src/. Needs a built tree (`make`), the
toolchain, and the `unicorn`, `pyelftools` and `numpy` packages (`make equiv` installs them into the ignored
.equiv-venv/). With -j, functions run in that many worker processes (each holds about 100 MB).

For each function it emulates only the VR4300's integer and FPU instructions, one function at a time (unicorn's
MIPS32 core; the game code is -mips3 -mgp32 -mfp32, so it uses no 64-bit registers):

- retail: the function's bytes from build/superbowling.elf at its address;
- draft: the unit compiled with its Makefile flags plus -DNON_MATCHING, the function cut from that object and
  relocated by hand: symbols resolve by name to the built ELF, the unit's .data/.bss to where the matching object
  was linked, and the draft's .rodata (with its jump tables) is copied to a scratch area.

Both run in the same RAM image: main's sections plus, for an overlay function, its own overlay, at their VRAMs;
a 2 MB heap of random words (pointers into the heap, small and edge integers, floats); pages of random words
mapped wherever else in kuseg random data points; a fresh stack. Each case
draws its arguments (by the prototype gcc's -aux-info gives) and, in most cases, randomises the bss (and in some
the .data) the function reads. Every callee is a stub: it records the target and its arguments (only the ones its
prototype declares, masked to their width; printf-style variadics by their format string), clobbers the
caller-saved registers and returns values from a generator seeded by the case and the call's position, the same
for both runs. Compared: the call sequence with arguments, the memory written between consecutive calls and after
the last (outside the function's own frame; a store of the value already there is no write), the return value by
the declared type, and the callee-saved registers. Pointers into a .rodata range compare by the bytes they point
to, pointers into the frame as STACK, so the draft's different string and frame addresses are not differences.

The function's own instructions run under a code hook that decodes each load and store before it executes
(unicorn's memory hooks corrupt MIPS delay slots); callees are `jr ra` stubs seen by the block hook. When the
retail run faults (an unaligned or kseg2 access, a jump to a non-function) or runs away (instruction or call
limit) on random data, the world is repaired and the case rerun: the loaded word that was the bad base becomes a
heap pointer (or a function entry), or a large loaded number becomes small. A case that still faults or runs
away in both runs is skipped; one where only one run does is a difference. Prints EQUIVALENT (n cases, k
skipped) or DIFFERS with the first differing case; exits 1 if any function differs or no case completed.
"""

import argparse
import collections
import gc
import multiprocessing
import os
import random
import re
import shlex
import struct
import subprocess
import sys
import tempfile

try:
    import numpy as np
    from elftools.elf.elffile import ELFFile
    from elftools.elf.relocation import RelocationSection
    from unicorn import Uc, UcError, UC_ARCH_MIPS, UC_MODE_MIPS32, UC_MODE_BIG_ENDIAN, UC_HOOK_CODE, \
        UC_HOOK_BLOCK
    from unicorn import mips_const as M
except ImportError as e:  # pragma: no cover
    print("equiv: missing Python package (%s); run `make equiv`" % e.name, file=sys.stderr)
    sys.exit(2)

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ELF_PATH = os.path.join(ROOT, "build", "superbowling.elf")
MAP_PATH = os.path.join(ROOT, "build", "superbowling.map")

# Emulated memory (kseg0; unicorn maps it onto physical 0).
RAM_SIZE = 0x01000000
IMAGE_END = 0x80400000          # game sections live below this
STACK_TOP = 0x803FF000
HEAP_LO, HEAP_HI = 0x80400000, 0x80600000
HEAP_PTR_HI = 0x80580000        # heap pointers leave 512 KB for member offsets
DRAFT_TEXT = 0x80C00000
DRAFT_RODATA = 0x80D00000
SENTINEL = 0x80FFF000           # return address of the tested call
FRAME_SPAN = 0x10000
MAX_INSNS = 150000
MAX_REPAIRS = 40
STUB = bytes.fromhex("03E0000800000000")

MAIN_SECTIONS = (".entry", ".main", ".main_bss", ".fat", ".knstatic", ".rdpfifo_bss")

A_REGS = [M.UC_MIPS_REG_A0, M.UC_MIPS_REG_A1, M.UC_MIPS_REG_A2, M.UC_MIPS_REG_A3]
S_REGS = [getattr(M, "UC_MIPS_REG_S%d" % i) for i in range(8)] + [M.UC_MIPS_REG_FP]
CLOBBER = [M.UC_MIPS_REG_AT, M.UC_MIPS_REG_V1] + A_REGS + \
    [getattr(M, "UC_MIPS_REG_T%d" % i) for i in range(10)]
MEM_SIZES = {0x20: 1, 0x21: 2, 0x22: 4, 0x23: 4, 0x24: 1, 0x25: 2, 0x26: 4, 0x28: 1, 0x29: 2, 0x2A: 4, 0x2B: 4,
             0x2E: 4, 0x31: 4, 0x35: 8, 0x37: 8, 0x39: 4, 0x3D: 8, 0x3F: 8}
GPR = [getattr(M, "UC_MIPS_REG_%d" % i) for i in range(32)]
STORE_OPS = {0x28, 0x29, 0x2A, 0x2B, 0x2E, 0x39, 0x3D, 0x3F}
UNALIGNED_OPS = {0x22, 0x26, 0x2A, 0x2E}
F_REGS = [getattr(M, "UC_MIPS_REG_F%d" % i) for i in range(32)]


# ----------------------------------------------------------------------------------------------- prototypes

INT_WIDTHS = {"u8": (8, False), "s8": (8, True), "char": (8, True), "unsigned char": (8, False),
              "signed char": (8, True), "u16": (16, False), "s16": (16, True), "short": (16, True),
              "unsigned short": (16, False), "short int": (16, True), "short unsigned int": (16, False)}
FLOATS = {"float": "f", "f32": "f", "double": "d", "f64": "d"}
LONGS = {"u64", "s64", "long long", "long long int", "unsigned long long", "long long unsigned int", "OSTime"}


def classify(t):
    """A parameter or return type as 'void', 'ptr', 'f', 'd', 'll', ('int', bits, signed) or '...'."""
    t = re.sub(r"\b(const|volatile|register|extern|static|struct|union|enum)\b", " ", t)
    t = " ".join(t.split())
    if t == "...":
        return "..."
    if "*" in t or "(" in t or "[" in t:
        return "ptr"
    if t == "void":
        return "void"
    if t in FLOATS:
        return FLOATS[t]
    if t in LONGS:
        return "ll"
    if t in INT_WIDTHS:
        return ("int",) + INT_WIDTHS[t]
    return ("int", 32, True)


def split_params(s):
    out, depth, cur = [], 0, ""
    for ch in s:
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        if ch == "," and depth == 0:
            out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out


def parse_aux(path):
    """Prototypes from gcc -aux-info: name -> (return class, [param classes], variadic)."""
    sigs = {}
    for line in open(path, errors="replace"):
        m = re.match(r"/\*[^*]*\*/ (?:extern |static )?(.*?)\b(\w+) \((.*)\);", line)
        if not m:
            continue
        ret, name, params = m.group(1).strip(), m.group(2), m.group(3)
        if name in sigs and ":NF" not in line:
            continue
        plist = [] if params.strip() in ("void", "") else split_params(params)
        variadic = bool(plist) and plist[-1] == "..."
        if variadic:
            plist = plist[:-1]
        sigs[name] = (classify(ret), [classify(p) for p in plist], variadic)
    return sigs


def slots(params):
    """o32 argument locations: per parameter ('fpr', n, kind) or ('slot', word index, kind)."""
    out, word, fp_ok, nfp = [], 0, True, 0
    for p in params:
        if p in ("d", "ll"):
            word = (word + 1) & ~1
        if p in ("f", "d") and fp_ok and nfp < 2 and word < 4:
            out.append(("fpr", 12 + 2 * nfp, p))
            nfp += 1
        else:
            fp_ok = False
            out.append(("slot", word, p))
        word += 2 if p in ("d", "ll") else 1
    return out


FMT_RE = re.compile(r"%[-+ #0]*(\*|\d+)?(?:\.(\*|\d+))?(hh|h|ll|l|L)?([diouxXcspfeEgGn%])")


def format_args(fmt):
    """Variadic argument classes a printf-style format consumes."""
    out = []
    for m in FMT_RE.finditer(fmt):
        width, prec, length, conv = m.groups()
        if conv == "%":
            continue
        if width == "*":
            out.append(("int", 32, True))
        if prec == "*":
            out.append(("int", 32, True))
        if conv in "feEgG":
            out.append("d")
        elif length == "ll":
            out.append("ll")
        else:
            out.append(("int", 32, True))
    return out


# ----------------------------------------------------------------------------------------------- build data

class Image:
    """The built ELF: sections, symbols, and the map's per-object section placements."""

    def __init__(self):
        self.f = open(ELF_PATH, "rb")
        self.elf = ELFFile(self.f)
        self.sections = {s.name: s for s in self.elf.iter_sections()}
        self.syms = {}
        allsyms = []
        for s in self.elf.get_section_by_name(".symtab").iter_symbols():
            if s.name.endswith(".NON_MATCHING") or not s.name:
                continue
            self.syms.setdefault(s.name, []).append(s)
            allsyms.append(s)
        self.place = {}
        self.rodata = []
        self.data_ranges = []
        for line in open(MAP_PATH):
            m = re.match(r"^ (\.text|\.data|\.bss|\.rodata)\s+0x([0-9a-f]+)\s+0x([0-9a-f]+) (\S+\.o\)?)$", line)
            if not m:
                continue
            sec, addr, size, obj = m.group(1), int(m.group(2), 16), int(m.group(3), 16), m.group(4)
            if addr < 0x80000000:
                continue
            self.place.setdefault(obj, {})[sec] = (addr, size)
            if sec == ".rodata" and size:
                self.rodata.append((addr, addr + size, obj))
            if sec == ".data" and size:
                self.data_ranges.append((addr, addr + size, obj))
        self.shndx_names = [s.name for s in self.elf.iter_sections()]
        # Callable entries: every symbol inside an object's .text (libultra's are OBJECT symbols of size 0),
        # named by its function symbol where there is one.
        text = sorted((a, a + n) for p in self.place.values() for sec, (a, n) in p.items() if sec == ".text" and n)
        starts = [lo for lo, hi in text]
        import bisect
        self.funcs = {}
        rank = {"STT_FUNC": 0, "STT_OBJECT": 1, "STT_NOTYPE": 2}
        for s in sorted(allsyms, key=lambda s: -rank.get(s["st_info"]["type"], 3)):
            a = s["st_value"]
            k = bisect.bisect_right(starts, a) - 1
            if k >= 0 and a < text[k][1] and s["st_info"]["type"] in rank and not s.name.startswith("."):
                self.funcs[a] = s.name

    def section_data(self, name):
        s = self.sections[name]
        if s["sh_type"] == "SHT_NOBITS":
            return s["sh_addr"], bytes(s["sh_size"])
        return s["sh_addr"], s.data()

    def func(self, name):
        for s in self.syms.get(name, []):
            if s["st_info"]["type"] == "STT_FUNC":
                return s
        raise KeyError(name)

    def lookup(self, name, near=None):
        cands = self.syms.get(name, [])
        if not cands:
            return None
        if near is not None and len(cands) > 1:
            same = [c for c in cands if c["st_shndx"] == near]
            if same:
                cands = same
        return cands[0]["st_value"]


def find_drafts():
    """(function, unit path) for every #ifdef NON_MATCHING draft in src/."""
    out = []
    for dirpath, _, files in os.walk(os.path.join(ROOT, "src")):
        for fn in sorted(files):
            if not fn.endswith(".c"):
                continue
            path = os.path.join(dirpath, fn)
            text = open(path, errors="replace").read()
            for m in re.finditer(r"#ifdef NON_MATCHING\n(.*?)#else\s*\nINCLUDE_ASM\(\"[^\"]+\", (\w+)\);", text,
                                 re.S):
                out.append((m.group(2), os.path.relpath(path, ROOT)))
    return sorted(out, key=lambda x: (x[1], x[0]))


def compile_draft(unit, workdir):
    """Compile UNIT with its Makefile command plus -DNON_MATCHING; returns (object path, aux-info path)."""
    obj = "build/" + unit[:-2] + ".o"
    cmd = subprocess.run(["make", "-n", "-B", obj], cwd=ROOT, capture_output=True, text=True).stdout
    line = [l for l in cmd.splitlines() if " -o %s " % obj in l][-1]
    out = os.path.join(workdir, "draft.o")
    aux = os.path.join(workdir, "aux.txt")
    line = line.replace(" -o %s " % obj, " -DNON_MATCHING -aux-info %s -o %s " % (shlex.quote(aux), out))
    r = subprocess.run(line, shell=True, cwd=ROOT, capture_output=True, text=True)
    if r.returncode:
        raise RuntimeError("draft compile failed:\n" + r.stderr)
    return out, aux, obj


def phys(v):
    """Unicorn's physical address of virtual V (kseg0 and kseg1 map onto 0; kuseg, under ERL, onto itself)."""
    return v & 0x1FFFFFFF if 0x80000000 <= v < 0xC0000000 else v


def virt(p):
    """The kseg0 address of physical P inside the emulated RAM (unicorn's memory hooks report physical ones)."""
    return p + 0x80000000 if p < RAM_SIZE else p


def sext16(v):
    return v - 0x10000 if v & 0x8000 else v


class Draft:
    """The draft function cut from its object and relocated against the built image."""

    def __init__(self, img, objpath, matched_obj, name, home_shndx):
        f = open(objpath, "rb")
        e = ELFFile(f)
        self.secs = list(e.iter_sections())
        symtab = e.get_section_by_name(".symtab")
        self.osyms = list(symtab.iter_symbols())
        text_idx = [i for i, s in enumerate(self.secs) if s.name == ".text"][0]
        tsyms = sorted((s["st_value"], s.name) for s in self.osyms
                       if s["st_shndx"] == text_idx and s["st_info"]["type"] in ("STT_FUNC", "STT_NOTYPE")
                       and s.name and not s.name.startswith("."))
        start = [v for v, n in tsyms if n == name][0]
        later = [v for v, n in tsyms if v > start]
        text = self.secs[text_idx].data()
        end = later[0] if later else len(text)
        self.start, self.end = start, end
        self.addr = DRAFT_TEXT
        place = img.place.get(matched_obj, {})
        bases = {}
        for i, s in enumerate(self.secs):
            if s.name in (".data", ".bss"):
                a, size = place.get(s.name, (None, 0))
                if s["sh_size"] and (a is None or size != s["sh_size"]):
                    raise RuntimeError("%s of the draft object (0x%x) differs from the matching object's" %
                                       (s.name, s["sh_size"]))
                bases[i] = a
            elif s.name == ".rodata":
                bases[i] = DRAFT_RODATA
            elif s.name == ".text":
                bases[i] = None
        self.rodata_bytes = b""
        self.text_idx = text_idx
        self.tsyms = tsyms
        self.img, self.home = img, home_shndx
        self.bases = bases
        code = bytearray(text)
        rod_idx = [i for i, s in enumerate(self.secs) if s.name == ".rodata"]
        rod = bytearray(self.secs[rod_idx[0]].data()) if rod_idx else bytearray()
        for rs in e.iter_sections():
            if not isinstance(rs, RelocationSection):
                continue
            target = self.secs[rs["sh_info"]].name
            if target == ".text":
                self._apply(list(rs.iter_relocations()), code, self.addr - start, (start, end))
            elif target == ".rodata":
                self._apply(list(rs.iter_relocations()), rod, DRAFT_RODATA, (0, len(rod)))
        self.code = bytes(code[start:end])
        self.rodata_bytes = bytes(rod)

    def _text_target(self, off):
        """Address of draft .text offset OFF: inside the draft itself, or the same-named function in the ELF."""
        if self.start <= off < self.end:
            return self.addr + off - self.start
        best = [(v, n) for v, n in self.tsyms if v <= off]
        v, n = best[-1]
        a = self.img.lookup(n, self.home)
        if a is None:
            raise RuntimeError("no ELF symbol for draft text symbol %s" % n)
        return a + off - v

    def _sym_value(self, sym):
        """(base address, is a .text section symbol)."""
        if sym["st_info"]["type"] == "STT_SECTION":
            idx = sym["st_shndx"]
            if idx == self.text_idx:
                return None, True
            return self.bases[idx], False
        a = self.img.lookup(sym.name, self.home)
        if a is None and sym["st_shndx"] not in ("SHN_UNDEF",) and sym["st_shndx"] in self.bases:
            if self.bases[sym["st_shndx"]] is not None:
                return self.bases[sym["st_shndx"]] + sym["st_value"], False
        if a is None:
            raise RuntimeError("unresolved symbol %s" % sym.name)
        return a, False

    def _apply(self, rels, buf, place_base, rng):
        lo, hi = rng
        pending = []
        for r in rels:
            off = r["r_offset"]
            typ = r["r_info_type"]
            sym = self.osyms[r["r_info_sym"]]
            ins = struct.unpack_from(">I", buf, off)[0]
            base, is_text = self._sym_value(sym)
            if typ == 5:  # HI16
                pending.append((off, ins, sym.name, base, is_text))
                continue
            if typ == 6:  # LO16
                his = [p for p in pending if p[2] == sym.name]
                hi_imm = (his[-1][1] & 0xffff) if his else 0
                ahl = (hi_imm << 16) + sext16(ins & 0xffff)
                tgt = (self._text_target(ahl) if is_text else base + ahl) & 0xffffffff
                for p in his:
                    if lo <= p[0] < hi:
                        h = ((tgt + 0x8000) >> 16) & 0xffff
                        struct.pack_into(">I", buf, p[0], (p[1] & 0xffff0000) | h)
                pending = [p for p in pending if p[2] != sym.name]
                if lo <= off < hi:
                    struct.pack_into(">I", buf, off, (ins & 0xffff0000) | (tgt & 0xffff))
                continue
            if not lo <= off < hi:
                continue
            if typ == 4:  # 26
                a = (ins & 0x3ffffff) << 2
                tgt = self._text_target(a) if is_text else base + a
                struct.pack_into(">I", buf, off, (ins & 0xfc000000) | ((tgt >> 2) & 0x3ffffff))
            elif typ == 2:  # 32
                tgt = self._text_target(ins) if is_text else base + ins
                struct.pack_into(">I", buf, off, tgt & 0xffffffff)
            else:
                raise RuntimeError("relocation type %d" % typ)


# ----------------------------------------------------------------------------------------------- random worlds

def rand_words(rng, n, small=False):
    """N random big-endian words: heap pointers, zero, small and edge integers, floats, raw bits."""
    kind = rng.integers(0, 100, n)
    ptr = (rng.integers(HEAP_LO, HEAP_PTR_HI, n, dtype=np.uint64) & ~np.uint64(7)).astype(np.uint32)
    smallv = rng.integers(0, 8, n).astype(np.uint32)
    neg = (np.uint32(0xFFFFFFFF) - rng.integers(0, 4, n).astype(np.uint32))
    flt = rng.uniform(-200.0, 200.0, n).astype(">f4").view(">u4").astype(np.uint32)
    raw = rng.integers(0, 1 << 32, n, dtype=np.uint64).astype(np.uint32)
    if small:
        w = np.where(kind < 50, 0, np.where(kind < 90, smallv, neg)).astype(np.uint32)
    else:
        w = np.where(kind < 35, ptr, np.where(kind < 55, 0, np.where(kind < 72, smallv, np.where(
            kind < 80, neg, np.where(kind < 92, flt, raw))))).astype(np.uint32)
    return w.astype(">u4").tobytes()


EDGE_INTS = [0, 1, 2, 3, 0xFFFFFFFF, 0x7FFFFFFF, 0x80000000, 0x7F, 0x80, 0xFF, 0x7FFF, 0x8000, 0xFFFF, 4, 5, 8,
             10, 16]
EDGE_FLOATS = [0.0, -0.0, 1.0, -1.0, 0.5, 100.0, -100.0, 1e6, 1e-6]


def gen_value(r, cls):
    """A Python random.Random draw for one argument or return value of class CLS (as a 32- or 64-bit int)."""
    if cls == "ptr":
        x = r.random()
        if x < 0.05:
            return 0
        return r.randrange(HEAP_LO, HEAP_PTR_HI) & ~7
    if cls == "f":
        v = r.choice(EDGE_FLOATS) if r.random() < 0.3 else r.uniform(-500, 500)
        return struct.unpack(">I", struct.pack(">f", v))[0]
    if cls == "d":
        v = r.choice(EDGE_FLOATS) if r.random() < 0.3 else r.uniform(-500, 500)
        return struct.unpack(">Q", struct.pack(">d", v))[0]
    if cls == "ll":
        return r.getrandbits(64) if r.random() < 0.5 else r.randrange(0, 1000)
    bits, signed = (32, True) if cls in ("void", "...") else cls[1:]
    x = r.random()
    if x < 0.35:
        v = r.choice(EDGE_INTS)
    elif x < 0.75:
        v = r.randrange(0, 12)
    elif x < 0.85:
        v = r.randrange(HEAP_LO, HEAP_PTR_HI) & ~7
    else:
        v = r.getrandbits(32)
    return extend(v, bits, signed)


def extend(v, bits, signed):
    v &= (1 << bits) - 1
    if signed and v & (1 << (bits - 1)):
        v -= 1 << bits
    return v & 0xffffffff


# ----------------------------------------------------------------------------------------------- the emulator

class Fault(Exception):
    pass


class Machine:
    def __init__(self, img, home_section):
        self.img = img
        self.base = bytearray(RAM_SIZE)
        names = list(MAIN_SECTIONS) + ([home_section, home_section + "_bss"] if home_section else [])
        self.bss = []
        self.loaded = []
        for n in names:
            if n not in img.sections:
                continue
            a, d = img.section_data(n)
            self.base[a - 0x80000000:a - 0x80000000 + len(d)] = d
            self.loaded.append((a, a + len(d)))
            if n.endswith("_bss") and len(d):
                self.bss.append((a, a + len(d)))
        self.fn = (0, 0)
        # Overlays share their VRAM and the asset files are linked at made-up ones: only the ranges of the main
        # segment's objects and of this overlay's count.
        overlays = {n[1:] for n in img.sections if n[1:] + "_bss" in {m[1:] for m in img.sections}} - {"main"}

        def mine(obj):
            if "/assets/" in obj:
                return False
            seg = [o for o in overlays if "/%s/" % o in obj]
            return not seg or seg == [home_section[1:]] if home_section else not seg

        self.rodata = [(lo, hi) for lo, hi, obj in img.rodata if mine(obj)]
        self.data_ranges = [(lo, hi) for lo, hi, obj in img.data_ranges if mine(obj)]
        # Every function entry in the image returns at once (jr ra; nop): the block hook records the call there.
        self.entries = {}
        for a in img.funcs:
            if 0x80000000 <= a < IMAGE_END and any(lo <= a < hi for lo, hi in self.loaded):
                self.base[a - 0x80000000:a - 0x80000000 + 8] = STUB
                self.entries[a] = 1
        self.entry_list = sorted(self.entries)

    # --- hooks
    # Unicorn's memory hooks (read, write, unmapped) corrupt MIPS delay slots: an access hooked in a delay slot
    # makes the branch target's first instruction run twice. So the function's own instructions run under a code
    # hook, which decodes each load and store before it executes, and only the block hook sees the stubs.
    def fresh(self):
        """A new CPU per run: unicorn's translation cache does not survive rewriting 16 MB of code and data.
        The old one's hooks are removed first: they reference this Machine, and the cycle would keep its 16 MB."""
        old = getattr(self, "uc", None)
        if old is not None:
            for h in self.hooks:
                old.hook_del(h)
            self.uc = old = None
            gc.collect()
        self.uc = Uc(UC_ARCH_MIPS, UC_MODE_MIPS32 + UC_MODE_BIG_ENDIAN)
        self.uc.mem_map(0, RAM_SIZE)
        self.hooks = [self.uc.hook_add(UC_HOOK_BLOCK, self._on_block),
                      self.uc.hook_add(UC_HOOK_CODE, self._on_code, begin=self.fn[0], end=self.fn[1] - 1)]
        self.lazy = set()
        self.loads = collections.deque(maxlen=512)
        self.bad = None
        self.store = None

    def _finish_store(self):
        """Record the bytes the previous instruction stored."""
        addr, n, old = self.store
        self.store = None
        new = bytes(self.uc.mem_read(phys(addr), n))
        tok = self.canon(int.from_bytes(new, "big"), raw=False) if n == 4 and not addr & 3 else None
        for i in range(n):
            a = addr + i
            if a not in self.seg:
                self.seg[a] = [old[i], None]
            self.seg[a][1] = (tok, i) if tok is not None else new[i]

    def _map_lazy(self, p):
        """Map the page of random words at physical P (the same for both runs of a case) where random data
        pointed; False if too many."""
        page = p & ~0xFFF
        if page in self.lazy:
            return True
        if len(self.lazy) >= 4096:
            return False
        self.uc.mem_map(page, 0x1000)
        self.uc.mem_write(page, rand_words(np.random.default_rng((self.case, page)), 0x400))
        for ra, (size, v) in self.repairs.items():
            if page <= phys(ra) < page + 0x1000:
                self.uc.mem_write(phys(ra), v.to_bytes(size, "big"))
        self.lazy.add(page)
        return True

    def _on_code(self, uc, addr, size, ud):
        if self.store:
            self._finish_store()
        if self.pending:
            for reg, v in self.pending:
                uc.reg_write(reg, v)
            self.pending = None
        ins = self.insns[(addr - self.fn[0]) >> 2]
        op = ins >> 26
        n = MEM_SIZES.get(op)
        if n is None:
            return
        base = (ins >> 21) & 31
        b = uc.reg_read(GPR[base]) & 0xffffffff if base else 0
        a = (b + sext16(ins & 0xffff)) & 0xffffffff
        if op in UNALIGNED_OPS:
            a &= ~3
        if a & (n - 1) or a >= 0xC0000000:
            self.bad = (a, b)
            self.fault = "%s access 0x%08X at 0x%08X" % ("unaligned" if a & (n - 1) else "unmapped", a, addr)
            uc.emu_stop()
            return
        p = phys(a)
        if p >= RAM_SIZE and not self._map_lazy(p):
            self.fault = "too many random pages"
            uc.emu_stop()
            return
        if op in STORE_OPS:
            if not self.sp0 - FRAME_SPAN <= a < self.sp0 + 0x40:
                self.store = (a, n, bytes(uc.mem_read(p, n)))
        elif n <= 4:
            self.loads.append((a, n, int.from_bytes(uc.mem_read(p, n), "big")))

    def _on_block(self, uc, addr, size, ud):
        if self.fn[0] <= addr < self.fn[1]:
            return
        if self.store:
            self._finish_store()
        if addr == SENTINEL:
            uc.emu_stop()
            self.done = True
            return
        if addr in self.entries:
            self.call(uc, addr)
            return
        if not self.fault:
            self.fault = "jump to 0x%08X, not a function" % addr
            self.bad = ("jump", addr)
            uc.emu_stop()

    def repair_loop(self):
        """For a run that hit the instruction or call limit: the most often reloaded large number becomes small."""
        count = collections.Counter()
        for addr, size, v in self.loads:
            sv = v - (1 << (8 * size)) if v >> (8 * size - 1) else v
            if abs(sv) >= 0x20 and not HEAP_LO <= v < HEAP_HI and not 0x80000000 <= v < IMAGE_END \
                    and addr not in self.repairs:
                count[(addr, size)] += 1
        if not count:
            return None
        (addr, size), _ = count.most_common(1)[0]
        return addr, size, self.rgen.randrange(0, 6)

    def repair(self):
        """A change to the world that removes this run's fault: the loaded word that was the faulting base
        becomes a heap pointer, or else the latest large loaded number becomes small. None if no candidate."""
        if self.bad is None:
            return None
        a, b = self.bad
        if a == "jump":
            for addr, size, v in reversed(self.loads):
                if size == 4 and v == b and addr not in self.repairs:
                    return addr, 4, self.rgen.choice(self.entry_list)
            return None
        for addr, size, v in reversed(self.loads):
            if size == 4 and v == b and addr not in self.repairs:
                return addr, 4, self.rgen.randrange(HEAP_LO, HEAP_PTR_HI) & ~7
        for addr, size, v in reversed(self.loads):
            sv = v - (1 << (8 * size)) if v >> (8 * size - 1) else v
            if abs(sv) >= 0x100 and not HEAP_LO <= v < HEAP_HI and not 0x80000000 <= v < IMAGE_END \
                    and addr not in self.repairs:
                return addr, size, self.rgen.randrange(0, 4)
        return None

    def canon(self, v, raw=True):
        """A value as compared: pointers into .rodata by their bytes, into the frame as STACK."""
        for lo, hi in self.rodata:
            if lo <= v < hi:
                b = bytes(self.uc.mem_read(phys(v), min(32, 0x80000000 + RAM_SIZE - v)))
                return ("RO", b.split(b"\0")[0][:32])
        if self.sp0 - FRAME_SPAN <= v < self.sp0:
            return ("STACK",)
        return v if raw else None

    def flush(self):
        out = tuple(sorted((a, n) for a, (o, n) in self.seg.items() if n != o))
        self.seg = {}
        return out

    def arg_values(self, uc, sig):
        ret, params, variadic = sig
        locs = slots(params)
        vals = []
        sp = uc.reg_read(M.UC_MIPS_REG_SP)

        def word(i):
            if i < 4:
                return uc.reg_read(A_REGS[i]) & 0xffffffff
            return struct.unpack(">I", uc.mem_read(phys(sp + 4 * i), 4))[0]

        def fetch(loc):
            kind, n, cls = loc
            if kind == "fpr":
                if cls == "f":
                    return ("f", uc.reg_read(F_REGS[n]) & 0xffffffff)
                return ("d", (uc.reg_read(F_REGS[n + 1]) & 0xffffffff) << 32 | (uc.reg_read(F_REGS[n]) &
                                                                                 0xffffffff))
            if cls in ("d", "ll"):
                return (cls, word(n) << 32 | word(n + 1))
            v = word(n)
            if isinstance(cls, tuple):
                v = extend(v, cls[1], cls[2])
            return self.canon(v)

        for loc in locs:
            vals.append(fetch(loc))
        if variadic:
            fmt_loc = [l for l, p in zip(locs, params) if p == "ptr"]
            extra = []
            if fmt_loc:
                fp = word(fmt_loc[-1][1]) if fmt_loc[-1][0] == "slot" else 0
                try:
                    s = bytes(uc.mem_read(phys(fp), 256)).split(b"\0")[0].decode("latin-1")
                    extra = format_args(s)
                except UcError:
                    extra = []
            w = (locs[-1][1] + (2 if locs[-1][2] in ("d", "ll") else 1)) if locs else 0
            for cls in extra:
                if cls in ("d", "ll"):
                    w = (w + 1) & ~1
                vals.append(fetch(("slot", w, cls)))
                w += 2 if cls in ("d", "ll") else 1
        return tuple(vals)

    def call(self, uc, addr):
        name = self.img.funcs.get(addr, "0x%08X" % addr)
        sig = self.sigs.get(name)
        if sig is None:
            args = tuple(self.canon(uc.reg_read(r) & 0xffffffff) for r in A_REGS)
        else:
            args = self.arg_values(uc, sig)
        self.calls.append((name, args, self.flush()))
        r = self.retgen(len(self.calls))
        rcls = sig[0] if sig else ("int", 32, True)
        v = gen_value(r, "ptr" if rcls == "ptr" else ("int", 32, True)) if rcls != "void" else r.getrandbits(32)
        out = [(reg, r.getrandbits(32)) for reg in CLOBBER]
        out.append((M.UC_MIPS_REG_V0, v))
        fv = gen_value(r, "d")
        out.append((F_REGS[0], gen_value(r, "f") if rcls != "d" else fv & 0xffffffff))
        out.append((F_REGS[1], fv >> 32))
        self.pending = out
        if len(self.calls) > 4000:
            self.fault = "call limit"
            uc.emu_stop()
            return

    # --- one run
    def run(self, code, addr, extra, sig, sigs, world, case, repairs):
        self.fn = (addr, addr + len(code))
        self.insns = struct.unpack(">%dI" % (len(code) // 4), code)
        self.fresh()
        uc = self.uc
        self.case = case
        self.repairs = repairs
        self.rgen = random.Random(case * 31 + len(repairs))
        uc.mem_write(0, bytes(world))
        for ra, (size, v) in repairs.items():
            if phys(ra) < RAM_SIZE:
                uc.mem_write(phys(ra), v.to_bytes(size, "big"))
        uc.mem_write(addr - 0x80000000, code)
        for a, b in extra:
            uc.mem_write(a - 0x80000000, b)
        self.sigs = sigs
        self.sp0 = STACK_TOP
        self.seg, self.calls = {}, []
        self.fault, self.done = None, False
        self.pending = None
        self.retgen = lambda i: random.Random((case << 20) ^ i)
        r = random.Random(case * 7919 + 1)
        for i in range(1, 32):
            reg = getattr(M, "UC_MIPS_REG_%d" % i)
            uc.reg_write(reg, r.getrandbits(32))
        for f in F_REGS:
            uc.reg_write(f, r.getrandbits(32))
        saved = [r.getrandbits(32) for _ in S_REGS]
        for reg, v in zip(S_REGS, saved):
            uc.reg_write(reg, v)
        ret, params, variadic = sig
        for kind, n, cls in slots(params):
            v = gen_value(r, cls)
            if kind == "fpr":
                uc.reg_write(F_REGS[n], v & 0xffffffff)
                if cls == "d":
                    uc.reg_write(F_REGS[n + 1], v >> 32)
            elif cls in ("d", "ll"):
                for k, w in ((n, v >> 32), (n + 1, v & 0xffffffff)):
                    if k < 4:
                        uc.reg_write(A_REGS[k], w)
                    else:
                        uc.mem_write(phys(self.sp0 + 4 * k), struct.pack(">I", w))
            elif n < 4:
                uc.reg_write(A_REGS[n], v)
            else:
                uc.mem_write(phys(self.sp0 + 4 * n), struct.pack(">I", v))
        uc.reg_write(M.UC_MIPS_REG_SP, self.sp0)
        self.args0 = [uc.reg_read(r) & 0xffffffff for r in A_REGS]
        uc.reg_write(M.UC_MIPS_REG_RA, SENTINEL)
        try:
            uc.emu_start(addr, SENTINEL, count=MAX_INSNS)
        except UcError as e:
            self.fault = self.fault or str(e)
        if not self.fault and uc.reg_read(M.UC_MIPS_REG_PC) == SENTINEL:
            self.done = True
        if self.store and self.done:
            self._finish_store()
        if self.fault == "call limit":
            return ("timeout", self.fault, len(self.calls), self.repair_loop())
        if self.fault:
            return ("fault", self.fault, len(self.calls), self.repair())
        if not self.done:
            return ("timeout", "instruction limit", len(self.calls), self.repair_loop())
        out = {"calls": self.calls, "writes": self.flush()}
        if ret == "f":
            out["ret"] = ("f", uc.reg_read(F_REGS[0]) & 0xffffffff)
        elif ret == "d":
            out["ret"] = ("d", (uc.reg_read(F_REGS[1]) & 0xffffffff) << 32 | uc.reg_read(F_REGS[0]) & 0xffffffff)
        elif ret == "ll":
            out["ret"] = (uc.reg_read(M.UC_MIPS_REG_V0) & 0xffffffff, uc.reg_read(M.UC_MIPS_REG_V1) & 0xffffffff)
        elif ret != "void":
            v = uc.reg_read(M.UC_MIPS_REG_V0) & 0xffffffff
            out["ret"] = self.canon(extend(v, ret[1], ret[2]) if isinstance(ret, tuple) else v)
        bad = [i for i, (reg, v) in enumerate(zip(S_REGS, saved)) if uc.reg_read(reg) & 0xffffffff != v]
        if uc.reg_read(M.UC_MIPS_REG_SP) != self.sp0:
            bad.append("sp")
        out["clobbered"] = tuple(bad)
        return ("ok", out)


def make_world(m, case, flavour):
    """The RAM image for one case: the game sections, a random heap and (by flavour) random bss and data."""
    rng = np.random.default_rng(case)
    w = bytearray(m.base)
    lo = HEAP_LO - 0x80000000
    w[lo:HEAP_HI - 0x80000000] = rand_words(rng, (HEAP_HI - HEAP_LO) // 4, small=(flavour == 3))
    if flavour in (1, 2, 3):
        for a, b in m.bss:
            w[a - 0x80000000:b - 0x80000000] = rand_words(rng, (b - a) // 4, small=(flavour != 1))
    if flavour == 1:
        for a, b in m.data_ranges:
            if a < IMAGE_END and (a & 3) == 0 and m.base[a - 0x80000000:b - 0x80000000].strip(b"\0"):
                w[a - 0x80000000:b - 0x80000000] = rand_words(rng, (b - a) // 4)
    return w


def diff_runs(a, b):
    """First difference between two completed runs, as text, or None."""
    if a.get("ret") != b.get("ret"):
        return "return value: retail %r, draft %r" % (a.get("ret"), b.get("ret"))
    for i, (x, y) in enumerate(zip(a["calls"], b["calls"])):
        if x[2] != y[2]:
            return "memory written before call %d (%s): %s" % (i, x[0], wdiff(x[2], y[2]))
        if x[0] != y[0]:
            return "call %d: retail calls %s, draft calls %s" % (i, x[0], y[0])
        if x[1] != y[1]:
            return "call %d (%s) arguments: retail %s, draft %s" % (i, x[0], fmt(x[1]), fmt(y[1]))
    if len(a["calls"]) != len(b["calls"]):
        n = min(len(a["calls"]), len(b["calls"]))
        return "call count: retail %d, draft %d (first extra: %s)" % (
            len(a["calls"]), len(b["calls"]), (a["calls"] + b["calls"])[n][0] if True else "")
    if a["writes"] != b["writes"]:
        return "memory written after the last call: %s" % wdiff(a["writes"], b["writes"])
    if a["clobbered"] != b["clobbered"]:
        return "callee-saved registers not restored: retail %r, draft %r" % (a["clobbered"], b["clobbered"])
    return None


def fmt(args):
    return "(" + ", ".join("0x%08X" % v if isinstance(v, int) else repr(v) for v in args) + ")"


def wdiff(x, y):
    dx, dy = dict(x), dict(y)
    keys = sorted(set(dx) | set(dy))
    for k in keys:
        if dx.get(k) != dy.get(k):
            return "first at 0x%08X: retail %r, draft %r (%d vs %d bytes written)" % (
                k, dx.get(k, "-"), dy.get(k, "-"), len(dx), len(dy))
    return "?"


def describe(img, name):
    """The retail function's section name (its overlay) and bytes."""
    s = img.func(name)
    sec = img.shndx_names[s["st_shndx"]]
    a0, data = img.section_data(sec)
    off = s["st_value"] - a0
    return sec, s["st_value"], data[off:off + s["st_size"]], s["st_shndx"]


def test_function(img, name, unit, cases, seed, verbose):
    sec, addr, retail, shndx = describe(img, name)
    home = sec if sec not in MAIN_SECTIONS else None
    with tempfile.TemporaryDirectory() as tmp:
        objpath, aux, matched_obj = compile_draft(unit, tmp)
        sigs = parse_aux(aux)
        draft = Draft(img, objpath, matched_obj, name, shndx)
    if name not in sigs:
        raise RuntimeError("no prototype for %s" % name)
    m = Machine(img, home)
    m.rodata.append((DRAFT_RODATA, DRAFT_RODATA + max(1, len(draft.rodata_bytes))))
    extra = [(DRAFT_RODATA, draft.rodata_bytes)] if draft.rodata_bytes else []
    return compare(m, (retail, addr, []), (draft.code, draft.addr, extra), sigs[name], sigs, cases, seed, verbose)


def compare(m, retail, draft, sig, sigs, cases, seed, verbose):
    """Run CASES random cases of both versions; returns (ok count, skipped, first difference or None)."""
    ok = skipped = 0
    for i in range(cases):
        case = seed * 1000003 + i
        world = make_world(m, case, i % 4)
        repairs = {}
        while True:
            a = m.run(retail[0], retail[1], retail[2], sig, sigs, world, case, repairs)
            if a[0] == "ok" or a[3] is None or len(repairs) >= MAX_REPAIRS:
                break
            repairs[a[3][0]] = a[3][1:]
        b = m.run(draft[0], draft[1], draft[2], sig, sigs, world, case, repairs)
        if a[0] == "ok" and b[0] == "ok":
            d = diff_runs(a[1], b[1])
            if d:
                return ok, skipped, "case %d: %s" % (i, d)
            ok += 1
        elif a[0] == "fault" and b[0] == "fault" or a[0] == b[0] == "timeout":
            skipped += 1
        elif a[0] == "timeout" or b[0] == "timeout":
            skipped += 1
        else:
            return ok, skipped, "case %d: retail %r, draft %r" % (i, a[:2], b[:2])
        if verbose and i % 500 == 499:
            print("  %d cases, %d compared" % (i + 1, ok), flush=True)
    return ok, skipped, None


_IMAGE = None


def test_one(job):
    """One function, in a worker process: (report line, 1 if it differs or could not be tested)."""
    global _IMAGE
    name, unit, cases, seed, verbose = job
    if _IMAGE is None:
        _IMAGE = Image()
    try:
        ok, skipped, d = test_function(_IMAGE, name, unit, cases, seed, verbose)
    except RuntimeError as e:
        return "%-24s ERROR %s" % (name, e), 1
    if d:
        return "%-24s DIFFERS after %d equal cases: %s" % (name, ok, d), 1
    if ok == 0:
        return "%-24s NO CASES (%d skipped: every case faulted or ran away)" % (name, skipped), 1
    return "%-24s EQUIVALENT (%d cases, %d skipped)" % (name, ok, skipped), 0


def main(argv):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("funcs", nargs="*")
    ap.add_argument("--cases", type=int, default=2000)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("-j", "--jobs", type=int, default=1)
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args(argv)
    if not os.path.isfile(ELF_PATH):
        print("equiv: %s missing; run make" % ELF_PATH, file=sys.stderr)
        return 2
    drafts = find_drafts()
    if args.funcs:
        known = dict(drafts)
        missing = [f for f in args.funcs if f not in known]
        if missing:
            print("equiv: no NON_MATCHING draft for %s" % ", ".join(missing), file=sys.stderr)
            return 2
        drafts = [(f, known[f]) for f in args.funcs]
    if not drafts:
        print("equiv: no drafts to test", file=sys.stderr)
        return 2
    jobs = [(name, unit, args.cases, args.seed, args.verbose) for name, unit in drafts]
    status = 0
    with multiprocessing.Pool(min(args.jobs, len(jobs))) as pool:
        for line, bad in pool.imap(test_one, jobs):
            print(line, flush=True)
            status |= bad
    return status


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
