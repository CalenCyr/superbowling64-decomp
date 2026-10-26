#!/usr/bin/env python3
"""Fixture tests for tools/rules.py. Each fixture marks the [match] findings it must produce with
`// expect: Rnn [Rnn...]` on the offending line, and a function charged through a type, global or macro
with `// expect-pending` in its body. Every other line must be free of [match] findings."""
import json
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
ROOT = HERE.parent.parent


def run(fixture):
    out = subprocess.run([sys.executable, str(ROOT / "tools" / "rules.py"), str(fixture), "--json", "-"],
                         capture_output=True, text=True, cwd=ROOT)
    data = json.loads(out.stdout)
    relp = str(fixture.relative_to(ROOT))
    lines = fixture.read_text().split("\n")
    want, pend = set(), set()
    for n, ln in enumerate(lines, 1):
        m = re.search(r"//\s*expect:\s*((?:R\d+\s*)+)", ln)
        if m:
            want |= {(n, r) for r in m.group(1).split()}
        if "expect-pending" in ln:
            for k in range(n, 0, -1):
                f = re.match(r"\w[\w\s\*]*?\b(\w+)\s*\(", lines[k - 1])
                if f and not lines[k - 1].startswith(" "):
                    pend.add(f.group(1))
                    break
    got = {(f["line"], f["rule"]) for f in data["findings"] if f["file"] == relp and f["tier"] == "match"}
    failing = {r["function"] for r in data["failing"] if r["unit"] == str(Path(relp).with_suffix(""))}
    errors = []
    for line, rule in sorted(want - got):
        errors.append(f"missing {rule} at line {line}: {lines[line - 1].strip()}")
    for line, rule in sorted(got - want):
        errors.append(f"unexpected {rule} at line {line}: {lines[line - 1].strip()}")
    for fn in sorted(pend - failing):
        errors.append(f"{fn} should be failing a [match] rule (charged)")
    if not want and failing:
        errors.append(f"clean fixture has failing functions: {sorted(failing)}")
    exit_ok = (out.returncode == 1) == bool(want)
    if not exit_ok:
        errors.append(f"exit status {out.returncode}")
    return errors, len(want), len(pend)


def main():
    bad = 0
    for fx in sorted(HERE.glob("*.c")):
        errors, nw, npend = run(fx)
        status = "FAIL" if errors else "ok"
        print(f"{status:4} {fx.name}: {nw} expected [match] findings, {npend} charged functions")
        for e in errors:
            print(f"     {e}")
        bad += bool(errors)
    print(f"rules_tests: {'FAILED' if bad else 'all passed'} ({bad} failing fixture(s))")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
