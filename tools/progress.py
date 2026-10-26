#!/usr/bin/env python3
"""Print progress from an objdiff report. Bytes matched is objdiff's count; matched under the rules needs the rule
checker (`rules`) and is reported separately."""
import json, sys

r = json.load(open(sys.argv[1]))
def line(name, m):
    print(f"{name:<10} functions {m.get('matched_functions', 0)}/{m.get('total_functions', 0)}"
          f"  code {int(m.get('matched_code', 0))}/{int(m.get('total_code', 0))} bytes"
          f" ({m.get('matched_code_percent', 0):.2f}%)")
line("all", r["measures"])
for c in r.get("categories", []):
    line(c["id"], c["measures"])
