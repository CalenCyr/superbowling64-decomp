#!/usr/bin/env python3
"""Write objdiff.json: one unit per C file (base = build/, target = expected/), plus every asm-only code unit as
target-only, so progress counts the whole code image."""
import json
from pathlib import Path

units = []
for c in sorted(Path("src").rglob("*.c")):
    o = Path("build") / c.with_suffix(".o")
    units.append({
        "name": str(c.with_suffix("")),
        "base_path": str(o),
        "target_path": str(Path("expected") / o),
        "metadata": {"source_path": str(c), "progress_categories": ["libultra" if "libultra" in c.parts else "game"]},
    })
for s in sorted(Path("asm").rglob("*.s")):
    if "nonmatchings" in s.parts or "data" in s.parts or s.name in ("header.s",):
        continue
    o = Path("build") / s.with_suffix(".o")
    units.append({
        "name": str(s.with_suffix("")),
        "target_path": str(Path("expected") / o),
        "metadata": {"progress_categories": ["libultra" if "libultra" in s.parts else "game"]},
    })

config = {
    "$schema": "https://raw.githubusercontent.com/encounter/objdiff/main/config.schema.json",
    "custom_make": "make",
    "build_target": False,
    "watch_patterns": ["*.c", "*.h", "*.s"],
    "progress_categories": [{"id": "game", "name": "Game"}, {"id": "libultra", "name": "libultra"}],
    "units": units,
}
Path("objdiff.json").write_text(json.dumps(config, indent=2) + "\n")
print(f"objdiff.json: {len(units)} units")
