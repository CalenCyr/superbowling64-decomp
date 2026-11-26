#!/usr/bin/env bash
# Run the permuter unattended over several parked functions, one after another, and record each one's best score.
#
#   tools/permute-batch.sh <minutes-per-function> <jobs> <unit>:<func> [<unit>:<func> ...]
#   e.g. tools/permute-batch.sh 60 8 gacode/3E350:GA_PhysicsStep 13490:Score_DrawTeamBoard
#
# Each function is imported with tools/permute.sh and run for the given time in its own process group, which is
# then killed (a plain `timeout` leaves the permuter's workers running). Results stay in nonmatchings/<func>[-N]/
# (ignored); nonmatchings/batch-report.txt gets one line per function: base score, best score, best output dir.
# Output is a clue for a human or agent to turn into honest source, never source itself.
set -uo pipefail
[ $# -ge 3 ] || { echo "usage: $0 <minutes> <jobs> <unit>:<func>..." >&2; exit 2; }
mins=$1; jobs=$2; shift 2
root=$(cd "$(dirname "$0")/.." && pwd)
cd "$root"
mkdir -p nonmatchings
report=nonmatchings/batch-report.txt
for spec in "$@"; do
    unit=${spec%%:*}; func=${spec#*:}
    out=$(tools/permute.sh "$unit" "$func" 2>&1)
    dir=$(printf '%s\n' "$out" | sed -n 's/.*Imported into \(nonmatchings\/[^ ]*\).*/\1/p' | tail -1)
    if [ -z "$dir" ]; then echo "$(date +%F\ %T) $spec IMPORT FAILED" >> "$report"; continue; fi
    log=$dir/batch.log
    setsid .permuter-venv/bin/python3 tools/decomp-permuter/permuter.py "$dir" -j"$jobs" --best-only > "$log" 2>&1 &
    pid=$!
    # A work copy that does not compile (e.g. pycparser misprinting `sizeof( T ) - x` as a cast) ends the run at
    # once; record it rather than wait out the slot.
    sleep 60
    if grep -q 'Unable to compile' "$log"; then
        kill -KILL -- -"$pid" 2>/dev/null
        echo "$(date +%F\ %T) $spec COMPILE FAILED (see $log)" >> "$report"; continue
    fi
    sleep $((mins * 60 - 60))
    kill -TERM -- -"$pid" 2>/dev/null; sleep 5; kill -KILL -- -"$pid" 2>/dev/null
    base=$(grep -m1 -o 'base score = [0-9]*' "$log" | grep -o '[0-9]*$')
    best=$(ls -d "$dir"/output-* 2>/dev/null | sed 's/.*output-\([0-9]*\)-.*/\1/' | sort -n | head -1)
    bestdir=$(ls -d "$dir"/output-"$best"-* 2>/dev/null | head -1)
    echo "$(date +%F\ %T) $spec base=${base:-?} best=${best:-none} ${bestdir:-}" >> "$report"
done
echo "done: $report"
