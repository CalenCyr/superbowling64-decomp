#!/usr/bin/env python3
"""Rename a struct member and every access to it through that struct type, driven by the compiler.

    python3 tools/rename_member.py TYPE OLD NEW [--note "invented: ..."] [--dry-run]

1. The member OLD is renamed to NEW in TYPE's definition (`typedef struct ... { ... } TYPE;` or
   `struct TYPE { ... };`, found once under include/ and src/).
2. Every C file under src/ is checked with KMC gcc `-fsyntax-only`, once plain and once with -DNON_MATCHING (the
   parked drafts must keep compiling). Each "has no member named `OLD'" error names a file and line; the `.OLD` or
   `->OLD` token on that line is renamed. When a line has more such tokens than errors (another type with a member
   OLD on the same line), each token is tried alone and kept only if it removes an error without adding a
   "no member named `NEW'" one. Fewer tokens than errors (a macro that repeats its argument) renames them all.
3. Repeat until no error for OLD remains. Errors that exist before the rename are ignored.
4. Comments that name the member as `TYPE.OLD` are updated, and with --note a provenance line
   `TYPE.NEW<TAB><header><TAB><note>` is appended to config/names.txt.

The compiler sees only real accesses, so a same-named member of another type is never touched. Accesses spelled
through a macro body cannot be rewritten at the use line; the tool stops and lists them. The byte gate still
decides: run `make gate` afterwards (a rename never changes bytes).
"""
import argparse
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
GCC_DIR = str(ROOT / "tools" / "gcc_2.7.2")
FLAGS = ["-fsyntax-only", "-G0", "-mips3", "-mgp32", "-mfp32", "-nostdinc",
         "-I", "include", "-I", "src", "-I", str(ROOT / "lib" / "ultralib" / "include")]
VARIANTS = ([], ["-DNON_MATCHING"])
ERR = re.compile(r"^(.+?):(\d+): (?:structure|union) has no member named `(\w+)'$")


def strip_comments(line):
    return re.sub(r"/\*.*?(\*/|$)", " ", line).split("//")[0]


def find_definition(root, typename, old):
    """Return (path, line index) of OLD's declaration inside TYPE's definition."""
    close = re.compile(r"^\s*}\s*" + re.escape(typename) + r"\s*;")
    open_tag = re.compile(r"^\s*(?:typedef\s+)?(?:struct|union)\s+" + re.escape(typename) + r"\b[^;]*{")
    member = re.compile(r"[\s\*]" + re.escape(old) + r"\s*[\[;,:)]")
    hits = []
    for path in sorted(list((root / "include").rglob("*.h")) + list((root / "src").rglob("*.[ch]"))):
        lines = path.read_text().split("\n")
        for i, ln in enumerate(lines):
            start = None
            if close.match(ln):
                depth = 0
                for j in range(i, -1, -1):
                    depth += lines[j].count("}") - lines[j].count("{")
                    if depth == 0:
                        start, end = j, i
                        break
            elif open_tag.match(ln):
                depth = 0
                for j in range(i, len(lines)):
                    depth += lines[j].count("{") - lines[j].count("}")
                    if depth == 0:
                        start, end = i, j
                        break
            if start is None:
                continue
            depth = 0
            for k in range(start, end + 1):
                code = strip_comments(lines[k])
                if depth == 1 and member.search(" " + code):
                    hits.append((path, k))
                depth += lines[k].count("{") - lines[k].count("}")
    return hits


def compile_errors(root, files, old_new):
    """Map (path, line) -> {name: count} of "no member named" errors for the names in old_new."""
    def one(args):
        path, extra = args
        out = subprocess.run([f"{GCC_DIR}/gcc", f"-B{GCC_DIR}/", *FLAGS, *extra, str(path.relative_to(root))],
                             cwd=root, capture_output=True, text=True,
                             env={"COMPILER_PATH": GCC_DIR, "PATH": "/usr/bin:/bin"})
        return out.stderr
    errs = {}
    with ThreadPoolExecutor() as pool:
        for text in pool.map(one, [(f, v) for f in files for v in VARIANTS]):
            seen = {}
            for ln in text.split("\n"):
                m = ERR.match(ln.strip())
                if m and m.group(3) in old_new:
                    key = ((root / m.group(1)).resolve(), int(m.group(2)))
                    seen.setdefault(key, {}).setdefault(m.group(3), 0)
                    seen[key][m.group(3)] += 1
            # A line is checked in both variants; count it once (the larger count).
            for key, names in seen.items():
                cur = errs.setdefault(key, {})
                for n, c in names.items():
                    cur[n] = max(cur.get(n, 0), c)
    return errs


def tokens(line, old):
    return [m for m in re.finditer(r"(\.|->)\s*" + re.escape(old) + r"\b", line)]


def replace_at(line, match, old, new):
    s, e = match.span()
    return line[:s] + match.group(0)[:-len(old)] + new + line[e:]


def main(argv=None):
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("type")
    ap.add_argument("old")
    ap.add_argument("new")
    ap.add_argument("--note", help="provenance for config/names.txt, e.g. 'invented: ...'")
    ap.add_argument("--root", type=Path, default=ROOT, help="tree with include/ and src/ (default: the repo)")
    ap.add_argument("--dry-run", action="store_true", help="report the sites, then restore every file")
    a = ap.parse_args(argv)
    root = a.root.resolve()
    files = sorted((root / "src").rglob("*.c"))
    backup = {}

    def write(path, lines):
        backup.setdefault(path, path.read_text())
        path.write_text("\n".join(lines))

    hits = find_definition(root, a.type, a.old)
    if len(hits) != 1:
        print(f"error: {a.type}.{a.old}: found {len(hits)} declarations", file=sys.stderr)
        return 2
    if find_definition(root, a.type, a.new):
        print(f"error: {a.type} already has a member {a.new}", file=sys.stderr)
        return 2
    names = (a.old, a.new)
    before = compile_errors(root, files, names)

    def fresh(errs):
        out = {}
        for key, ns in errs.items():
            for n, c in ns.items():
                d = c - before.get(key, {}).get(n, 0)
                if d > 0:
                    out.setdefault(key, {})[n] = d
        return out

    hpath, hline = hits[0]
    lines = hpath.read_text().split("\n")
    # The declarator is the first OLD outside a comment on the line.
    text = lines[hline]
    for m in re.finditer(r"\b" + re.escape(a.old) + r"\b", text):
        if strip_comments(text[:m.end()]).rstrip().endswith(a.old):
            lines[hline] = text[:m.start()] + a.new + text[m.end():]
            break
    write(hpath, lines)
    sites, stuck = [], []
    for _ in range(50):
        errs = fresh(compile_errors(root, files, names))
        todo = {k: v[a.old] for k, v in errs.items() if v.get(a.old)}
        if not todo:
            break
        progress = False
        for (path, lineno), count in sorted(todo.items()):
            lines = path.read_text().split("\n")
            text = lines[lineno - 1]
            toks = tokens(text, a.old)
            # As many tokens as errors, or fewer when a macro repeats its argument: rename them all (a wrong one
            # shows up as a "no member named `NEW'" error in the final check).
            if 0 < len(toks) <= count:
                for m in reversed(toks):
                    text = replace_at(text, m, a.old, a.new)
                lines[lineno - 1] = text
                write(path, lines)
                sites.append((path, lineno, count))
                progress = True
                continue
            if not toks:
                stuck.append((path, lineno, text.strip()))
                continue
            # More tokens than errors: try each one alone.
            kept = 0
            for idx in range(len(toks) - 1, -1, -1):
                m = tokens(lines[lineno - 1], a.old)[idx]
                trial = list(lines)
                trial[lineno - 1] = replace_at(lines[lineno - 1], m, a.old, a.new)
                write(path, trial)
                scope = [path] if path.suffix == ".c" else files
                e2 = fresh(compile_errors(root, scope, names)).get((path, lineno), {})
                if e2.get(a.old, 0) < count - kept and e2.get(a.new, 0) == 0:
                    lines = trial
                    kept += 1
                else:
                    write(path, lines)
            if kept:
                sites.append((path, lineno, kept))
                progress = True
            else:
                stuck.append((path, lineno, text.strip()))
        if stuck or not progress:
            break
    errs = fresh(compile_errors(root, files, names))
    left = sorted(k for k, v in errs.items() if v.get(a.old) or v.get(a.new))
    if stuck or left:
        for path, lineno, text in stuck:
            print(f"unresolved: {path.relative_to(root)}:{lineno}: {text}", file=sys.stderr)
        for path, lineno in left:
            print(f"error left: {path.relative_to(root)}:{lineno}", file=sys.stderr)
        for path, text in backup.items():
            path.write_text(text)
        print("restored every file", file=sys.stderr)
        return 1
    # Comments naming the member as TYPE.OLD.
    pat = re.compile(r"\b" + re.escape(a.type) + r"\." + re.escape(a.old) + r"\b")
    for path in sorted(list((root / "include").rglob("*.h")) + list((root / "src").rglob("*.[ch]"))):
        text = path.read_text()
        if pat.search(text):
            backup.setdefault(path, text)
            path.write_text(pat.sub(f"{a.type}.{a.new}", text))
    rel = hpath.relative_to(root)
    print(f"{a.type}.{a.old} -> {a.new}: definition {rel}:{hline + 1}, "
          f"{sum(c for _, _, c in sites)} access(es) on {len(sites)} line(s)")
    for path, lineno, c in sites:
        print(f"  {path.relative_to(root)}:{lineno}" + (f" (x{c})" if c > 1 else ""))
    if a.dry_run:
        for path, text in backup.items():
            path.write_text(text)
        return 0
    if a.note:
        names_txt = root / "config" / "names.txt"
        if names_txt.exists():
            with names_txt.open("a") as f:
                f.write(f"{a.type}.{a.new}\t{hpath.name}\t{a.note}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
