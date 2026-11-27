#!/usr/bin/env python3
"""Self-test for tools/equiv.py: compiles sample.c in three variants with the project's compiler and checks that
the reference and an equivalent rewrite compare EQUIVALENT, and that a planted bug is reported as a difference.
Needs the toolchain and the .equiv-venv packages, not the ROM."""
import os
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
sys.path.insert(0, os.path.dirname(HERE))
import equiv  # noqa: E402

GCC = os.path.join(ROOT, "tools", "gcc_2.7.2")
STUBS = {"Sample_Get": 0x80100000, "Sample_Put": 0x80100010}
GLOBALS = {"sample_total": 0x80200000}


class FakeImage:
    """The parts of equiv.Image the comparison uses: one RAM section holding the stubs and the global."""

    def __init__(self):
        self.sections = {".main": None}
        self.funcs = {a: n for n, a in STUBS.items()}
        self.place = {}
        self.rodata = []
        self.data_ranges = []

    def section_data(self, name):
        return 0x80100000, bytes(0x200000)

    def lookup(self, name, near=None):
        return STUBS.get(name, GLOBALS.get(name))


def build(variant, tmp):
    out = os.path.join(tmp, "v%d.o" % variant)
    aux = os.path.join(tmp, "v%d.aux" % variant)
    env = dict(os.environ, COMPILER_PATH=GCC)
    subprocess.run([os.path.join(GCC, "gcc"), "-B" + GCC + "/", "-c", "-G0", "-mips3", "-mgp32", "-mfp32", "-O2",
                    "-Wa,-N", "-DVARIANT=%d" % variant, "-aux-info", aux, "-o", out,
                    os.path.join(HERE, "sample.c")], check=True, env=env, cwd=tmp)
    return out, aux


def main():
    img = FakeImage()
    with tempfile.TemporaryDirectory() as tmp:
        progs = []
        for v in range(3):
            obj, aux = build(v, tmp)
            d = equiv.Draft(img, obj, None, "Sample_Run", None)
            progs.append((d.code, d.addr, [(equiv.DRAFT_RODATA, d.rodata_bytes)] if d.rodata_bytes else []))
            sigs = equiv.parse_aux(aux)
    m = equiv.Machine(img, None)
    sig = sigs["Sample_Run"]
    errors = []
    if progs[0][0] == progs[1][0]:
        errors.append("variants 0 and 1 compiled to the same bytes; the test needs different code")
    ok, skipped, d = equiv.compare(m, progs[0], progs[1], sig, sigs, 300, 1, False)
    if d or ok < 100:
        errors.append("equivalent rewrite: %d cases, %s" % (ok, d))
    ok, skipped, d = equiv.compare(m, progs[0], progs[2], sig, sigs, 300, 1, False)
    if not d or "Sample_Put" not in d and "return" not in d and "memory" not in d:
        errors.append("planted bug not reported (%d cases, %s)" % (ok, d))
    for e in errors:
        print("equiv-test: FAIL: " + e)
    if not errors:
        print("equiv-test: OK (equivalent rewrite equal; planted bug found: %s)" % d)
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
