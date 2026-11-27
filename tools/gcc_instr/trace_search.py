#!/usr/bin/env python3
"""Search source spellings of one function by its register-allocation trace.

    trace_search.py <permdir> <func> [--source FILE] [--limit N] [--show K] [--vars a,b]

<permdir> is a tools/permute.sh import (base.c, target.o). --source (default <permdir>/base.c) may hold the
permuter's text macros PERM_GENERAL, PERM_ONCE and PERM_VAR (no random or AST macros); every expansion is
compiled once with the instrumented cc1 (tools/gcc_2.7.2_instr, objects identical to the official compiler's),
which gives both the object and the allocation trace. Per candidate it prints the differing instruction count
against target.o (relocations named, branch targets function-relative), then the user variables' pseudos with
their allocator (L: local-alloc in block b, G#k: global-alloc priority rank and priority) and final register,
and the global-alloc order of every pseudo of interest (--pseudos adds unnamed ones by regno). Diagnostic only.
"""
import argparse
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
sys.path.insert(0, os.path.join(ROOT, "tools", "decomp-permuter"))
from src.perm.parse import _rec_perm_parse  # noqa: E402
from src.perm.perm import RootPerm  # noqa: E402
from src.perm.perm import EvalState  # noqa: E402

GCC = os.path.join(ROOT, "tools", "gcc_2.7.2_instr", "bin")
FLAGS = ["-c", "-G0", "-mips3", "-mgp32", "-mfp32", "-O2", "-nostdinc",
         "-I", os.path.join(ROOT, "include"), "-I", os.path.join(ROOT, "src"),
         "-I", os.path.join(ROOT, "lib", "ultralib", "include")]


def disasm(obj, func):
    out = subprocess.run(["mips64-linux-gnu-objdump", "-dr", "--no-show-raw-insn", "-M", "reg-names=32",
                          "-j", ".text", obj], capture_output=True, text=True).stdout
    lines, shown, start, on = [], [], None, False
    funcs = {int(a, 16): b for a, b in re.findall(r"^([0-9a-f]+) <(.+)>:$", out, re.M)}
    pending_hi = []
    for ln in out.splitlines():
        m = re.match(r"^([0-9a-f]+) <(.+)>:$", ln)
        if m:
            on = m.group(2) == func
            start = int(m.group(1), 16)
            continue
        if not on:
            continue
        m = re.match(r"^\s+[0-9a-f]+: (R_MIPS_\w+)\s+(\S+)", ln)
        if m and lines:
            sym = re.sub(r"\+0x[0-9a-f]+$", "", m.group(2))
            if sym == ".rodata" or re.match(r"D_[0-9A-F]+", sym):
                sym = "<rodata>"  # string pool: named in the target, section-relative in a compile
                lines[-1] = re.sub(r",-?\d+(\(\w+\))?$", r",<lo>\1", lines[-1])
                lines[-1] = re.sub(r",0x[0-9a-f]+$", ",<hi>", lines[-1])
            if sym == ".text" and m.group(1) == "R_MIPS_26":
                sym = shown[-1]  # a static callee: the section-relative target objdump resolved
            elif sym == ".text":
                # a static function's address: %lo carries the offset; its %hi takes the name of the next %lo
                if m.group(1) == "R_MIPS_HI16":
                    pending_hi.append(len(lines) - 1)
                    lines[-1] += " @"
                    continue
                imm = int(re.search(r",(-?\d+)(\(\w+\))?$", lines[-1]).group(1))
                sym = funcs.get(imm, ".text+%d" % imm)
                lines[-1] = re.sub(r",-?\d+(\(\w+\))?$", r",0\1", lines[-1])
                for i in pending_hi:
                    lines[i] += sym
                pending_hi.clear()
            lines[-1] += " @" + sym
            continue
        m = re.match(r"^\s+([0-9a-f]+):\s+(.*)$", ln)
        if m:
            ins = m.group(2).split("\t")
            text = " ".join(ins[:2])
            text = re.sub(r"\b[0-9a-f]+ <[^>+]+\+0x([0-9a-f]+)>", r"+\1", text)
            n = re.search(r"\b[0-9a-f]+ <([^>]+)>", text)
            shown.append(n.group(1) if n else "")
            text = re.sub(r"\b[0-9a-f]+ <[^>]+>", "", text)
            lines.append(text.strip())
    return lines


def diff_count(a, b):
    import difflib
    sm = difflib.SequenceMatcher(None, a, b, autojunk=False)
    return sum(max(i2 - i1, j2 - j1) for tag, i1, i2, j1, j2 in sm.get_opcodes() if tag != "equal")


def parse_trace(path, func):
    text = open(path).read()
    blocks = re.split(r"^=== ", text, flags=re.M)
    body = next((b for b in blocks if b.startswith(func + " ")), "")
    names, pinfo, local, gorder, homes = {}, {}, {}, [], {}
    for ln in body.splitlines():
        m = re.match(r"V (?:parm|var) (\S+)(?: depth=\d+)? r(\d+)$", ln)
        if m:
            names.setdefault(int(m.group(2)), m.group(1))
        if ln.startswith("P pseudos"):
            for r, refs, live, calls, bb in re.findall(r"r(\d+):(\d+)/(\d+)/(\d+)/b(-?\d+)", ln):
                pinfo[int(r)] = (int(refs), int(live), int(calls), int(bb))
        m = re.match(r"L block (\d+) ", ln)
        if m:
            cur = int(m.group(1))
        m = re.match(r"L   q\d+ regs=(\S+) ", ln)
        if m:
            for r in re.findall(r"r(\d+)", m.group(1)):
                local[int(r)] = cur
        m = re.match(r"G   #(\d+) a\d+ regs=((?:r\d+ )+)refs=(\d+) live=(\d+) .*?pri=(\d+)", ln)
        if m:
            regs = [int(x) for x in re.findall(r"r(\d+)", m.group(2))]
            gorder.append((regs, int(m.group(3)), int(m.group(4)), int(m.group(5))))
        if ln.startswith("F final homes:"):
            for r, h in re.findall(r"r(\d+)=(\S+)", ln):
                homes[int(r)] = h
    return names, pinfo, local, gorder, homes


def describe(names, pinfo, local, gorder, homes, extra):
    rank = {}
    for k, (regs, refs, live, pri) in enumerate(gorder):
        for r in regs:
            rank[r] = (k, pri, refs, live)
    want = sorted(set(names) | set(extra))
    parts = []
    for r in want:
        nm = names.get(r, "r%d" % r)
        if r in rank:
            k, pri, refs, live = rank[r]
            how = "G#%d pri=%d %d/%d" % (k, pri, refs, live)
        elif r in local:
            how = "L b%d" % local[r]
        else:
            how = "-"
        parts.append("%s=r%d[%s]%s" % (nm, r, how, homes.get(r, "")))
    return " ".join(parts)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("permdir")
    ap.add_argument("func")
    ap.add_argument("--source")
    ap.add_argument("--limit", type=int, default=2000)
    ap.add_argument("--pseudos", default="", help="extra regnos to describe, comma separated")
    ap.add_argument("--best", type=int, default=0, help="print only candidates at or below this diff count")
    ap.add_argument("--print-source", action="store_true", help="print the function of each shown candidate")
    ap.add_argument("--show-diff", action="store_true", help="print each shown candidate's differing lines")
    ap.add_argument("--only", type=int, help="evaluate just this candidate")
    ap.add_argument("--keep-knobs", action="store_true",
                    help="pass GCC_ALLOC_FORCE_GLOBAL/GCC_ALLOC_GLOBAL_ORDER through (to check the scorer)")
    args = ap.parse_args()

    src = open(args.source or os.path.join(args.permdir, "base.c")).read()
    perm = RootPerm(_rec_perm_parse(src))
    if perm.is_random():
        sys.exit("random perm macros are not supported")
    target = disasm(os.path.join(args.permdir, "target.o"), args.func)
    if not target:
        sys.exit("function not in target.o")
    extra = [int(x) for x in args.pseudos.split(",") if x]
    env = dict(os.environ, COMPILER_PATH=GCC)
    if not args.keep_knobs:
        env.pop("GCC_ALLOC_FORCE_GLOBAL", None)
        env.pop("GCC_ALLOC_GLOBAL_ORDER", None)
    count = min(perm.perm_count, args.limit)
    print("candidates: %d of %d" % (count, perm.perm_count))
    with tempfile.TemporaryDirectory() as tmp:
        c, o, tr = (os.path.join(tmp, n) for n in ("cand.c", "cand.o", "cand.trace"))
        best = None
        for seed in ([args.only] if args.only is not None else range(count)):
            state = EvalState()
            text = perm.evaluate(seed, state)
            if state.ast_perms:
                sys.exit("AST perm macros (PERM_ORDER, PERM_LINESWAP...) are not supported: use PERM_GENERAL")
            open(c, "w").write(text)
            if os.path.exists(tr):
                os.unlink(tr)
            env["GCC_ALLOC_TRACE"] = tr
            res = subprocess.run([os.path.join(GCC, "gcc"), "-B" + GCC + "/"] + FLAGS + [c, "-o", o],
                                 env=env, capture_output=True, text=True, cwd=tmp)
            if res.returncode != 0:
                print("%4d: compile error: %s" % (seed, res.stderr.strip().splitlines()[-1:]))
                continue
            mine = disasm(o, args.func)
            n = diff_count(target, mine)
            if best is None or n < best[0]:
                best = (n, seed)
            if args.best and n > args.best:
                continue
            info = describe(*parse_trace(tr, args.func), extra)
            print("%4d: diff=%d %s" % (seed, n, info))
            if args.show_diff:
                import difflib
                for ln in list(difflib.unified_diff(target, mine, lineterm="", n=0))[2:]:
                    print("      " + ln)
            if args.print_source:
                m = re.search(r"\n[^\n]*\b%s\s*\([^;]*?\{.*?\n\}" % re.escape(args.func), text, re.S)
                print(m.group(0) if m else "(function not found)")
        if best:
            print("best: diff=%d at candidate %d" % best)


if __name__ == "__main__":
    main()
