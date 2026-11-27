#!/usr/bin/env python3
"""mod_ld.py -- write the linker script `make mod` links with.

    tools/mod_ld.py SCRIPT OUT [--packed DIR --objdir DIR --ld LD]

Copies SCRIPT without the retail pad (build/assets/pad.o) and starts each asset segment on a 16-byte ROM boundary
(PI DMA needs an even ROM address; the gap is 0). With --packed, the asset segments come from the files tools/assets.py
pack wrote to DIR: each is turned into an object in OBJDIR (ld -r -b binary) that the script links in place of the
split assets/*.bin, so the ROM layout, the BW64FAT entries (_ROM_START/_ROM_END symbols) and IDX follow their sizes.
"""

import argparse
import os
import re
import shlex
import subprocess
import sys

SEGS = ("idx", "str", "bgmptr", "bgmwbk", "septr", "sewbk", "bfx")


def main(argv):
    ap = argparse.ArgumentParser()
    ap.add_argument("script")
    ap.add_argument("out")
    ap.add_argument("--packed")
    ap.add_argument("--objdir")
    ap.add_argument("--ld", default="mips64-linux-gnu-ld")
    args = ap.parse_args(argv)
    text = open(args.script).read()
    text = "".join(l for l in text.splitlines(True) if not re.match(r"^ *build/assets/pad\.o\(", l))
    text = re.sub(r"^( *(?:%s)_ROM_END = __romPos;)$" % "|".join(SEGS), r"\1 __romPos = ALIGN(__romPos, 16);",
                  text, flags=re.M)
    if args.packed:
        os.makedirs(args.objdir, exist_ok=True)
        for seg in SEGS:
            src = os.path.join(args.packed, "%-8sSYS.bin" % seg.upper())
            if not os.path.isfile(src):
                print("mod: %s missing: run tools/assets.py extract and pack" % src, file=sys.stderr)
                return 1
            data = open(src, "rb").read()
            binf = os.path.join(args.objdir, seg + ".bin")
            with open(binf, "wb") as f:
                f.write(data)
            subprocess.check_call(shlex.split(args.ld) + ["-r", "-b", "binary", "-o", seg + ".o", seg + ".bin"], cwd=args.objdir)
            new = text.replace("build/assets/%s.o(" % seg, os.path.join(args.objdir, seg + ".o") + "(")
            if new == text:
                print("mod: %s not found in the linker script" % seg, file=sys.stderr)
                return 1
            text = new
    with open(args.out, "w") as f:
        f.write(text)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
