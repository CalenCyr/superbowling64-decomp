# Instrumented cc1: register-allocation trace

`build.sh` fetches the source of the project's compiler, the decompals
[mips-gcc-2.7.2](https://github.com/decompals/mips-gcc-2.7.2) tag `v0.1` (the release `tools/gcc_2.7.2/` comes
from; GPL), applies `allocation-trace.patch` and builds `cc1` into the ignored `tools/gcc_2.7.2_instr/`.
`tools/gcc_2.7.2_instr/bin/` is a compiler directory like `tools/gcc_2.7.2/`: the official driver, `cpp` and `as`,
with the rebuilt `cc1`. The fetched source and the binaries are never committed.

```sh
tools/gcc_instr/build.sh          # fetch, patch, build (needs gcc -m32; fetches glibc-devel.i686 if missing)
tools/gcc_instr/build.sh verify   # every C unit, every NON_MATCHING draft and ultralib: objects must be identical
```

`verify` must print `differ=0` before any trace is trusted. The patch only prints; it never changes a decision.
The build stays on the official toolchain: this compiler is a diagnostic.

## Tracing

Set `GCC_ALLOC_TRACE` to a file (appended to) or `-` (stderr), and compile with `tools/gcc_2.7.2_instr/bin`:

```sh
G=tools/gcc_2.7.2_instr/bin
GCC_ALLOC_TRACE=/tmp/x.trace COMPILER_PATH=$G $G/gcc -B$G/ -S -G0 -mips3 -mgp32 -mfp32 -O2 -nostdinc \
    -I include -I src -I lib/ultralib/include -DNON_MATCHING -o /tmp/x.s src/gacode/3E350.c
```

Per function (`=== name`), with pseudo numbers as in the `-dl`/`-dg` RTL dumps (which land in the current
directory):

| prefix | content |
|---|---|
| `V` | each parameter and user variable and its pseudo (`r82`), or `mem` |
| `P` | every pseudo: refs (weighted by loop depth), live length, calls crossed, block (`b-2`: global) |
| `L` | local-alloc: block-local pseudos left to global-alloc and why; ties (`tie r83 <- r82`: a copy whose source dies); hard-register suggestions from copies; per block each qty's regs, class, refs, birth/death, calls, priority, suggestions; the suggested-pass and priority orders; each `find` with the registers live over the qty's range and the result |
| `G` | global-alloc: allocnos in priority order with refs, live length, calls, priority (`floor_log2(refs) * refs / live * 10000`) and conflicts; each `find` with hard conflicts, copy preferences, preferences and registers others prefer, and the reason for the choice (first free in pass 0/1, copy preference, preference, eviction of a local-alloc pseudo) |
| `R`, `F` | reload: spilled hard registers and where their pseudos went; final home of every pseudo (register, stack, or a rematerialised constant/memory equivalence) and the spill registers |

Allocation order is plain register order on MIPS (no `REG_ALLOC_ORDER`): `$2 $3 $4 ...`, then `$16`-`$23`, `$30`
for call-crossing pseudos.

Diagnostic knob (changes code, for experiments only): `GCC_ALLOC_FORCE_GLOBAL=FUNCTION:REGNO[,REGNO]` leaves those
block-local pseudos to global-alloc, to test whether one allocator decision explains a register difference.
`GCC_ALLOC_GLOBAL_ORDER=FUNCTION:REGNO,REGNO[,...]` refills the priority-order slots those pseudos' allocnos hold
with the pseudos in the given order (`F:73,83` swaps two neighbours; listing a high-priority pseudo moves the others
up to its slot), to test whether a different global-alloc priority order explains a register difference; the trace
prints `G order forced` when it applies. Neither knob is ever part of the build.
