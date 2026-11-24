#!/usr/bin/env python3
"""Fixture test for tools/rename_member.py: renames Foo_t.unk_04 in a copy of tree/ and checks each line marked
`/* want N keep M */` (N accesses renamed, M of another type's unk_04 kept), the definition and the comment."""
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
TOOL = HERE.parent / "rename_member.py"


def main():
    errors = []
    with tempfile.TemporaryDirectory() as tmp:
        root = Path(tmp) / "tree"
        shutil.copytree(HERE / "tree", root)
        (root / "config").mkdir()
        (root / "config" / "names.txt").write_text("")
        out = subprocess.run([sys.executable, str(TOOL), "Foo_t", "unk_04", "count", "--root", str(root),
                              "--note", "invented: test"], capture_output=True, text=True)
        if out.returncode != 0:
            errors.append(f"exit {out.returncode}: {out.stderr.strip()}")
        for ln in (root / "src" / "a.c").read_text().split("\n"):
            m = re.search(r"/\* want (\d+) keep (\d+) \*/", ln)
            if m:
                code = ln[:m.start()]
                got = (len(re.findall(r"\bcount\b", code)), len(re.findall(r"\bunk_04\b", code)))
                if got != (int(m.group(1)), int(m.group(2))):
                    errors.append(f"want {m.group(1)} keep {m.group(2)}, got {got}: {ln.strip()}")
        header = (root / "include" / "t.h").read_text()
        if "int count;" not in header or "Foo_t.count in a comment" not in header \
                or "int unk_04;              /* keep" not in header:
            errors.append("definition or comment not updated as expected")
        if (root / "config" / "names.txt").read_text() != "Foo_t.count\tt.h\tinvented: test\n":
            errors.append("names.txt line missing")
        # A second rename to an existing member must be refused.
        out = subprocess.run([sys.executable, str(TOOL), "Foo_t", "unk_00", "count", "--root", str(root)],
                             capture_output=True, text=True)
        if out.returncode != 2:
            errors.append("rename onto an existing member was not refused")
    for e in errors:
        print(f"FAIL {e}")
    print(f"rename_member_tests: {'FAILED' if errors else 'all passed'}")
    return 1 if errors else 0


if __name__ == "__main__":
    sys.exit(main())
