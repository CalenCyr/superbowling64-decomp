#!/usr/bin/env bash
# Merge a worker lane into main after the project's checks, and print one summary line.
#
#   tools/merge-lane.sh <worktree> <branch>
#
# In the worktree: refuse a dirty tree, rebase the branch onto main, regenerate asm, run the gate, the rule
# checker and the tool tests, and scan the diff for SDK copyright text. Then fast-forward main (this repository),
# regenerate asm, gate again, refresh expected/ and print the report headline. It does not push and does not
# touch README.md; the worktree is left detached at main and the branch deleted.
set -uo pipefail
[ $# -eq 2 ] || { echo "usage: $0 <worktree> <branch>" >&2; exit 2; }
wt=$1; br=$2
main=$(cd "$(dirname "$0")/.." && pwd)
fail() { echo "MERGE FAILED ($br): $*"; exit 1; }
step() { "$@" >/dev/null 2>&1 || fail "$*"; }

cd "$wt" || fail "no worktree $wt"
[ -z "$(git status --porcelain)" ] || fail "dirty tree in $wt"
git rebase -q main >/dev/null 2>&1 || { git rebase --abort >/dev/null 2>&1; fail "rebase conflict"; }
rm -rf asm
step make split
make gate 2>&1 | grep -q '^GATE OK' || fail "gate red in $wt"
step make rules
step make rules-test
step make rename-test
step make n64crc-test
if git diff main | grep -iqE 'silicon graphics|nintendo co|copyright'; then fail "SDK/copyright text in diff"; fi
files=$(git diff --stat main | tail -1)

cd "$main" || fail "no main repo"
git merge -q --ff-only "$br" >/dev/null 2>&1 || fail "ff merge"
rm -rf asm
step make split
make gate 2>&1 | grep -q '^GATE OK' || fail "gate red on main after merge"
step make expected
head=$(make report 2>&1 | grep -E '^all +matched' | sed 's/  */ /g')
git -C "$wt" checkout -q --detach main && git branch -q -d "$br"
echo "MERGED $br at $(git rev-parse --short HEAD): $files | $head"
