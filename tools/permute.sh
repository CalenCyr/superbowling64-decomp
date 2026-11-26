#!/usr/bin/env bash
# Import a function into decomp-permuter (tools/decomp-permuter, git submodule) and optionally run it.
#
#   tools/permute.sh <unit> <func> [permuter.py args...]
#   e.g. tools/permute.sh gscode/5EC00 func_80118040_gscode -j8 --stop-on-zero
#
# <unit> is the path under src/ without .c. The import preprocesses src/<unit>.c with NON_MATCHING and PERMUTER
# defined (so a parked `#ifdef NON_MATCHING` draft is the base and INCLUDE_ASM lines vanish), compiles it with KMC
# gcc 2.7.2 and the unit's flags from the Makefile, and assembles asm/nonmatchings/<unit>/<func>.s with modern GAS (scored through mips64-linux-gnu-objdump, the permuter's MIPS default).
# The import keeps the whole unit (--no-prune): pruning turns the unit's earlier functions into declarations, which
# hides the inlining that the -O3 units and `inline` helpers depend on. Work directories go to nonmatchings/<func>[-N]/ (ignored). With no extra arguments only the import is done; with
# arguments, permuter.py runs on the new directory with them.
#
# Python requirements (no sudo): a venv at .permuter-venv (ignored), created on first use with `toml` and
# `Levenshtein` from PyPI. Override the interpreter with PERMUTER_PYTHON.
set -euo pipefail

if [ $# -lt 2 ]; then
    echo "usage: $0 <unit> <func> [permuter.py args...]" >&2
    exit 2
fi
unit=$1
func=$2
shift 2

root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"

permuter=tools/decomp-permuter
if [ ! -f "$permuter/import.py" ]; then
    echo "$permuter is missing: run git submodule update --init $permuter" >&2
    exit 1
fi

c_file=src/$unit.c
asm_file=asm/nonmatchings/$unit/$func.s
[ -f "$c_file" ] || { echo "no $c_file" >&2; exit 1; }
[ -f "$asm_file" ] || { echo "no $asm_file (run make split)" >&2; exit 1; }

py=${PERMUTER_PYTHON:-}
if [ -z "$py" ]; then
    venv=.permuter-venv
    if [ ! -x "$venv/bin/python3" ]; then
        python3 -m venv "$venv"
        "$venv/bin/pip" install -q toml Levenshtein
    fi
    py=$venv/bin/python3
fi

# The unit's optimisation level: target-specific CFLAGS apply only to the object's own recipe, so read a dry run.
opt=$(make -n --no-print-directory "build/src/$unit.o" -W "$c_file" 2>/dev/null \
    | grep -o -- ' -O[0-9]*' | tail -1 | tr -d ' ')
[ -n "$opt" ] || { echo "could not read the unit's -O level from the Makefile" >&2; exit 1; }

mkdir -p nonmatchings
settings=nonmatchings/.settings-$func-$$.toml
cat > "$settings" <<EOF
compiler_type = "gcc"
compiler_command = "env COMPILER_PATH=tools/gcc_2.7.2 tools/gcc_2.7.2/gcc -Btools/gcc_2.7.2/ -c -G0 -mips3 -mgp32 -mfp32 -O2 $opt -nostdinc -I include -I src -I lib/ultralib/include"
assembler_command = "mips64-linux-gnu-as -EB -march=vr4300 -mabi=32 -G0 -I include"
EOF

# import.py preprocesses with the system `cpp`, which rejects gbi.h's `&##ucode##TextStart` paste in gSPLoadUcodeL
# (KMC's cpp only warns; KMC's cpp can't take import.py's variadic -D macros). Put a `cpp` first on PATH that
# searches a copy of PR/gbi.h with `&##` spelled `& ` first: the same tokens, and only the work copy sees it.
shadow=$root/nonmatchings/.cpp-shadow
mkdir -p "$shadow/PR" "$shadow/bin"
sed 's/&##/\& /g' lib/ultralib/include/PR/gbi.h > "$shadow/PR/gbi.h"
syscpp=$(PATH=$(printf '%s' "$PATH" | tr ':' '\n' | grep -vxF "$shadow/bin" | paste -sd:) command -v cpp)
printf '#!/bin/sh\nexec %s -I %s "$@"\n' "$syscpp" "$shadow" > "$shadow/bin/cpp"
chmod +x "$shadow/bin/cpp"

# Take the work directory from import.py's own "Imported into <dir>" line: a listing diff picks the wrong one when
# two imports run at the same time.
out=$(PATH=$shadow/bin:$PATH "$py" "$permuter/import.py" --no-prune --settings "$settings" "$c_file" "$asm_file")
rm -f "$settings"
echo "$out"
dir=$(printf '%s\n' "$out" | sed -n 's/.*Imported into \(nonmatchings\/[^ ]*\).*/\1/p' | tail -1)
[ -n "$dir" ] || { echo "import made no directory" >&2; exit 1; }

# With --no-prune the candidate object holds the whole unit, and the default objdump scores all of its .text
# against the one-function target, so shorter (broken) candidates win. Disassemble only the target function.
printf 'objdump_command = "mips64-linux-gnu-objdump -drz -m mips:4300 --disassemble=%s"\n%s\n' \
    "$func" "$(cat "$dir/settings.toml")" > "$dir/settings.toml"

# The permuter compiles every candidate with the bodies of all other non-`inline` functions reduced to declarations.
# In an -O3 unit that removes the implicit inlining of the unit's earlier functions, so mark the functions defined
# before the target `inline` in the (disposable) work copy; at -O3 gcc 2.7.2 inlines them the same way.
if [ "$opt" = "-O3" ]; then
    "$py" - "$dir/base.c" "$func" <<'EOF'
import re, sys
path, func = sys.argv[1], sys.argv[2]
lines = open(path).read().split("\n")
for i, line in enumerate(lines):
    m = re.match(r"^(?!inline\b|static inline\b|typedef\b|extern\b)[A-Za-z_][\w \*]*?\b(\w+)\s*\(.*\)\s*\{$", line)
    if m:
        if m.group(1) == func:
            break
        lines[i] = "inline " + line
open(path, "w").write("\n".join(lines))
EOF
fi

if [ $# -gt 0 ]; then
    exec "$py" "$permuter/permuter.py" "$dir" "$@"
fi
echo "run: $py $permuter/permuter.py $dir -j8"
