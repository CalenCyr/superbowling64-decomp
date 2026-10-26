#!/usr/bin/env python3
"""Print progress from an objdiff report. Bytes matched is objdiff's count.

    python3 tools/progress.py report.json [--rules build/rules.json]

With `--rules` (the JSON from `tools/rules.py --json`) it first prints the headline, matched under the rules:
objdiff-exact functions minus the MATCH-PENDING ones (exact but failing a [match] rule). ORIGINAL-ASM functions
are left out of both the matched count and the total."""
import argparse, json

ap = argparse.ArgumentParser()
ap.add_argument("report")
ap.add_argument("--rules", help="rules JSON from tools/rules.py --json")
args = ap.parse_args()

r = json.load(open(args.report))

if args.rules:
    rules = json.load(open(args.rules))
    failing = {(f["unit"], f["function"]) for f in rules["failing"]}
    orig = {(f["unit"], f["function"]) for f in rules["functions"] if f["kind"] == "original-asm"}
    groups = {"all": [0, 0, []]}
    for u in r.get("units", []):
        cats = ["all"] + u.get("metadata", {}).get("progress_categories", [])
        for f in u.get("functions", []):
            key = (u["name"], f["name"])
            if key in orig:
                continue
            exact = f.get("fuzzy_match_percent") == 100.0
            for c in cats:
                g = groups.setdefault(c, [0, 0, []])
                g[1] += 1
                if exact and key not in failing:
                    g[0] += 1
                elif exact:
                    g[2].append(key)
    for name, (ok, total, pending) in groups.items():
        pct = 100.0 * ok / total if total else 0.0
        print(f"{name:<10} matched under the rules {ok}/{total} functions ({pct:.2f}%), "
              f"{len(pending)} MATCH-PENDING")
    if orig:
        print(f"ORIGINAL-ASM (excluded): {len(orig)}")

def line(name, m):
    print(f"{name:<10} functions {m.get('matched_functions', 0)}/{m.get('total_functions', 0)}"
          f"  code {int(m.get('matched_code', 0))}/{int(m.get('total_code', 0))} bytes"
          f" ({m.get('matched_code_percent', 0):.2f}%)")
line("all", r["measures"])
for c in r.get("categories", []):
    line(c["id"], c["measures"])
