#!/usr/bin/env python3
"""Count source idioms per unit across the matched C sources.

Usage: python3 tools/idioms.py [--out report.md] [src/<unit>.c ...]

Reads src/**/*.c (not src/libkmc), drops the `#ifdef NON_MATCHING` branch of each parked
function, and counts the spellings that matching work showed to affect code generation:
loop forms, increment spellings, local types, declaration order, chained assignments,
if/else polarity, early returns, `?:`, pointer walking vs indexing, `void *` parameters.
Prints a short summary; --out writes the per-unit markdown table. Counts are regex-based
and approximate: they are priors for choosing between spellings, not proofs.
"""
import re, sys, glob, os, collections

PATTERNS = {
    "for(;;)": r"\bfor\s*\(",
    "while": r"\bwhile\s*\((?![^;]*\)\s*;)",
    "do-while": r"\bdo\s*\{?",
    "for up i<n": r"for\s*\([^;]*;\s*\w+\s*<\s*[^;]+;\s*[^)]*\+\+",
    "for down": r"for\s*\([^;]*;[^;]*;\s*(?:\w+--|--\w+|\w+\s*-=)",
    "for i!=n": r"for\s*\([^;]*;\s*\w+\s*!=",
    "i++": r"(?<![\w+])\w+\+\+\s*[;)]",
    "++i": r"(?<![\w])\+\+\w+\s*[;)]",
    "+= 1": r"\+=\s*1\s*[;)]",
    "-= 1/--": r"(?:-=\s*1\s*[;)]|\w--\s*[;)]|--\w+\s*[;)])",
    "s16 local": r"^\s+(?:register\s+)?s16\s+\w+",
    "u16 local": r"^\s+u16\s+\w+",
    "s32 local": r"^\s+s32\s+\w+",
    "u8 local": r"^\s+u8\s+\w+",
    "int local": r"^\s+int\s+\w+",
    "s16 += step": r"\b\w+\s*\+=\s*\w+\s*;",
    "chained a=b=x": r"\w+\s*=\s*\w+\s*=\s*[^=;]+;",
    "if (!x) fail": r"\bif\s*\(\s*!",
    "else": r"\belse\b",
    "else if": r"\belse\s+if\b",
    "early return": r"\breturn\b[^;]*;\s*\n\s*\}\s*\n",
    "?:": r"\?[^;]*:[^;]*;",
    "ptr walk *p++": r"\*\s*\w+\+\+|\*\+\+\w+|\w+\+\+\s*->",
    "index a[i]": r"\w+\[\s*\w+\s*\]",
    "void * param": r"\(\s*[^)]*void\s*\*\s*\w+",
    "switch": r"\bswitch\s*\(",
    "inline helper": r"\binline\b",
    "layout coord": r"\b(?:left|top|right|bottom|x0|y0|x1|y1)\b",
    "cast (u8*)/(s16*)": r"\(\s*(?:u8|s8|u16|s16|u32|s32)\s*\*\s*\)",
}

def strip(text):
    out, skip, depth = [], False, 0
    for ln in text.split("\n"):
        s = ln.strip()
        if s.startswith("#ifdef NON_MATCHING"):
            skip, depth = True, 1
            continue
        if skip:
            if s.startswith("#if"):
                depth += 1
            elif s.startswith("#else") and depth == 1:
                skip = False
            elif s.startswith("#endif"):
                depth -= 1
                if depth == 0:
                    skip = False
            continue
        out.append(ln)
    text = "\n".join(out)
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)

def decl_order(text):
    """Per function: are loop counters (i, j, k, n, row, col) declared first or last among locals?"""
    first = last = 0
    for m in re.finditer(r"\)\s*\{\n((?:[ \t]+[A-Za-z_][^\n;{}()]*;[ \t]*\n)+)", text):
        names = [re.findall(r"(\w+)\s*(?:=[^;]*)?;", l)[-1] for l in m.group(1).strip().split("\n") if re.findall(r"(\w+)\s*(?:=[^;]*)?;", l)]
        ctr = [k for k, n in enumerate(names) if n in ("i", "j", "k", "n", "row", "col", "idx")]
        if len(names) > 1 and ctr:
            if ctr[0] == 0: first += 1
            if ctr[-1] == len(names) - 1: last += 1
    return first, last

def main():
    args = sys.argv[1:]
    out = None
    if "--out" in args:
        k = args.index("--out"); out = args[k + 1]; del args[k:k + 2]
    files = args or [f for f in sorted(glob.glob("src/**/*.c", recursive=True)) if "/libkmc/" not in f]
    table = {}
    tot = collections.Counter()
    for f in files:
        text = strip(open(f).read())
        c = collections.Counter()
        for k, p in PATTERNS.items():
            c[k] = len(re.findall(p, text, re.M))
        c["counters first"], c["counters last"] = decl_order(text)
        c["lines"] = text.count("\n")
        table[f] = c
        tot.update(c)
    keys = list(PATTERNS) + ["counters first", "counters last", "lines"]
    if out:
        with open(out, "w") as o:
            o.write("# Idioms per unit\n\n| unit | " + " | ".join(keys) + " |\n|" + "---|" * (len(keys) + 1) + "\n")
            for f, c in table.items():
                o.write(f"| {f[4:]} | " + " | ".join(str(c[k]) for k in keys) + " |\n")
            o.write("| TOTAL | " + " | ".join(str(tot[k]) for k in keys) + " |\n")
    print("totals:", ", ".join(f"{k}={tot[k]}" for k in keys))

if __name__ == "__main__":
    main()
