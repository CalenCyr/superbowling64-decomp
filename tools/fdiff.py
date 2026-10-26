#!/usr/bin/env python3
"""Side-by-side disassembly of one function: build/ (left) vs expected/ (right).
Usage: tools/fdiff.py <unit .o path under build/> <function>"""
import difflib, re, subprocess, sys

def disasm(obj, sym):
    out = subprocess.run(["mips64-linux-gnu-objdump", "-dr", "--no-show-raw-insn", "-M", "no-aliases",
                          f"--disassemble={sym}", obj], capture_output=True, text=True).stdout
    lines = []
    for l in out.splitlines():
        m = re.match(r"\s+([0-9a-f]+):\s+(.*)", l)
        if m:
            lines.append(f"{int(m.group(1), 16):4x}: {m.group(2).strip()}")
        elif "R_MIPS" in l:
            lines[-1] += "  ; " + l.split()[-1]
    return lines

obj, sym = sys.argv[1], sys.argv[2]
a, b = disasm(obj, sym), disasm("expected/" + obj, sym)
if a == b:
    print(f"{sym}: identical ({len(a)} instructions)")
    sys.exit(0)
for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes():
    for k in range(max(i2 - i1, j2 - j1)):
        l = a[i1 + k] if i1 + k < i2 else ""
        r = b[j1 + k] if j1 + k < j2 else ""
        print(f"{' ' if tag == 'equal' else '|'} {l:<55} {r}")
sys.exit(1)
