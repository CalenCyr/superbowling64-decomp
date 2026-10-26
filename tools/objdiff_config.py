#!/usr/bin/env python3
"""Write objdiff.json: one unit per C file (base = build/, target = expected/), one per libultra archive member that
superbowling.yaml links (base = build/ultralib/, target = expected/), plus every asm-only code unit as target-only,
so progress counts the whole code image. `metadata.complete` (ByteMatch) marks units fully built from source."""
import json, re
from pathlib import Path

ULTRALIB = Path("build/ultralib/K/libgultra_rom/src")
ULTRALIB_SRC = Path("lib/ultralib/src")

units = []
for c in sorted(Path("src").rglob("*.c")):
    o = Path("build") / c.with_suffix(".o")
    units.append({
        "name": str(c.with_suffix("")),
        "base_path": str(o),
        "target_path": str(Path("expected") / o),
        "metadata": {
            "source_path": str(c),
            "complete": "INCLUDE_ASM(" not in c.read_text(),
            "progress_categories": ["libultra" if "libultra" in c.parts else "game"],
        },
    })
# splat `lib` subsegments name archive members by basename; gu/rotate.o and mgu/rotate.o assemble to identical bytes.
members = sorted(set(re.findall(r"\blib, libgultra_rom, (\w+)|\bobject: (\w+)", Path("superbowling.yaml").read_text())))
for name in sorted({a or b for a, b in members}):
    o = sorted(ULTRALIB.rglob(name + ".o"))[0]
    rel = o.relative_to(ULTRALIB).with_suffix("")
    src = next((p for p in (ULTRALIB_SRC / rel.with_suffix(ext) for ext in (".c", ".s")) if p.exists()), None)
    metadata = {"complete": True, "progress_categories": ["libultra"]}
    if src:
        metadata["source_path"] = str(src)
    units.append({
        "name": "libultra/" + str(rel),
        "base_path": str(o),
        "target_path": str(Path("expected") / o),
        "metadata": metadata,
    })
for s in sorted(Path("asm").rglob("*.s")):
    if "nonmatchings" in s.parts or "data" in s.parts or s.name in ("header.s",):
        continue
    o = Path("build") / s.with_suffix(".o")
    units.append({
        "name": str(s.with_suffix("")),
        "target_path": str(Path("expected") / o),
        "metadata": {"complete": False, "progress_categories": ["libultra" if "libultra" in s.parts else "game"]},
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
