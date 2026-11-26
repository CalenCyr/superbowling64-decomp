# Super Bowling 64 (USA, NBWE): agent guide

Reconstruct original source so the original compiler, with the original flags, emits object code that matches
the retail binary byte for byte, function by function. Correct structure outranks any score: classes, types,
ownership, storage, control flow, calling conventions and referents.

The point of the project is source a reader can understand. Matching bytes prove the source is faithful; they
don't make it readable. Every function must pass the reader test of the `decomp-code-rules` skill: would a
programmer on the original team have typed this? Source that matches through tricks (a fakematch) is not a
match.

This file is project-independent. Fill in the **Project adapter** section once per repository. The skills
(`matcher`, `wall-identifier`, `helper-recovery`, `permute`, `orchestrator`) use the generic verbs defined there.

## Project adapter (fill in per repository)

| verb | meaning | this project's command |
|---|---|---|
| `build` | compile every unit, compare every annotated function | `make` (then `make report` for per-function status) |
| `match <unit>` | compile and compare one unit quickly (the inner loop) | `make build/src/<unit>.o && python3 tools/fdiff.py build/src/<unit>.o <func>` |
| `diff <function>` | side-by-side disassembly: compiled vs retail | `make diff UNIT=src/<unit> FUNC=<func>` (text); `tools/objdiff/objdiff-cli diff -u src/<unit> <func>` (TUI) |
| `report` | refresh the progress report (objdiff or similar) | `make report` (objdiff 3.8.2; targets are `expected/`, refreshed by `make expected` only on a green gate) |
| `gate` | the checks that must stay green before a commit or merge | `make gate` (about 2 s): deletes `build/`, rebuilds, checks SHA-1 `56937a9e9536af55ebed06480413265848c4a04a`; success line starts `GATE OK` |
| `disasm` / `xref` / `class` | retail assembly, callers and callees, vtable and hierarchy evidence | `make split` (splat) → `asm/`, `assets/` |
| `permute <function>` | the permuter, if there is one | `tools/permute.sh <unit> <func> [permuter.py args]` (decomp-permuter, submodule `tools/decomp-permuter`; e.g. `tools/permute.sh gscode/5EC00 GS_TeamSelect -j8 --best-only`). Imports the unit with `NON_MATCHING` defined and its Makefile `-O` level into ignored `nonmatchings/<func>/`; Python deps (`toml`, `Levenshtein`) go in an ignored `.permuter-venv/` made on first use. An explicit `inline` the draft lacks must be added to the work copy's `base.c`. Only the target function is disassembled for scoring (the work copy keeps the whole unit); several imports may run at once. `timeout` on a run does not stop the permuter's worker processes: kill them by PID afterwards. Scores include a constant penalty where splat names an interior address (`D_8005C0D4` for `cont_noteGameName[12]`), so a true match can score above 0 |
| `merge` / `permute-batch` | merge a worker lane after all checks; run the permuter unattended | `tools/merge-lane.sh <worktree> <branch>` (rebase, split, gate, rules, tool tests, SDK scan, ff-merge, gate, `make expected`; one summary line; no push). `tools/permute-batch.sh <minutes> <jobs> <unit>:<func>...` (each function in its own process group, killed after the time; best scores in ignored `nonmatchings/batch-report.txt`; output is a clue, never source) |
| `rules [<unit>]` | the rule checker: [match] findings (`MATCH-PENDING`) and advisory findings (naming, readability) | `make rules [UNIT=src/<unit>]` (`tools/rules.py`; `--report report.json` limits MATCH-PENDING to exact functions, `--json` feeds `make report`'s headline; allow list `config/rules.txt`; its tests: `make rules-test`). It can't see Section F patterns that need the reader test (see its docstring) |
| `rename <type> <member>` | rename a struct member and exactly its accesses through that type (compiler-driven: the build's "no member named" errors, plain and `-DNON_MATCHING`, locate the sites) | `python3 tools/rename_member.py TYPE OLD NEW [--note "invented: ..."] [--dry-run]` (appends the `config/names.txt` line; updates `TYPE.OLD` in comments; refuses an access spelled through a macro body and restores every file; its test: `make rename-test`); `make gate` afterwards |

Also record:
- **Toolchain (this project):** KMC GCC 2.7.2 (N64 SDK), user ruling. Evidence: epilogues place `addiu sp` in the
  `jr ra` delay slot throughout, including the libultra region (KMC `libgultra`); splat also detects KMC.
  Compiler: `tools/gcc_2.7.2/` (decompals build of KMC gcc 2.7.2 + KMC as 2.6). Linker and objcopy: system
  `mips64-linux-gnu-*` (binutils 2.46), `ld -m elf32btsmip`.
- **Flags** (verified on five game functions): `-c -G0 -mips3 -mgp32 -mfp32 -O2`. `-O3` gave the same bytes on those
  (no static callees); switch a unit to `-O3` only on evidence of inlining. Plain `char` is signed: retail byte
  strings use `lbu`, so they are `u8`.
  - Per-unit override: `src/libkmc/*.c` (KMC's C runtime) build with `-O` instead of `-O2` (Makefile). Evidence: its
    code is unscheduled; `rand` stores its state before computing the result and `func_80056690` loads its stack
    arguments in source order, which `-O2`'s scheduler reorders; every libkmc C function matches at `-O`.
  - Per-unit override: units `3AD0`, `4890` and `4DC0` build with `-O3` (Makefile). Evidence: they inline their own
    earlier functions at the call sites without `inline` (gcc 2.7.2's `-O3` turns on `-finline-functions`); each
    fails the gate at `-O2`. Unit `53B0` also builds with `-O3`: `Obj_FreeAll` holds the bodies of `Obj_Free`
    and `Obj_ClearList` and fails at plain `-O2`; `-O2` with `inline` on those two matches as well, so the bytes don't
    decide, and the unit follows the other units' `-O3`. Overlay unit `gscode/58F40` also builds with `-O3`: `GS_Frame` inlines the unit's earlier `GS_PushScreen`/`GS_PopScreen`, and fails both at `-O2` and with `inline` at `-O2`. Overlay unit `gscode/5EC00` builds with `-O3` too: retail `GS_TeamSelect` holds `GS_UnlockMask`'s body twice. Overlay unit `opcode/6CC50` builds with `-O3` as well: `OP_PakScreen` holds `OP_ConvertPakChar`'s body at its two call sites; `inline` on `OP_ConvertPakChar` at `-O2` matches too, so the bytes don't decide, and the unit follows 53B0's choice. Unit `2DF0` builds with `-O3` too: `Cont_Thread` holds the bodies of `Rumble_Probe`, `Rumble_Start` and `Rumble_Stop`. Unit `166B0` instead matches at `-O2` with `inline` on `Snd_SetFxType` and fails at
    `-O3`.
- **C pipeline:** a C unit is a splat `c` subsegment (`src/<unit>.c`); unmatched functions stay as
  `INCLUDE_ASM("asm/nonmatchings/<unit>", <func>);`. C is compiled by the KMC `gcc` driver, so KMC `as` 2.6
  assembles both compiled code and included asm (modern GAS expands `move` to `or`, KMC to `addu`). Asm-only units
  use modern GAS. `make split` never overwrites an existing `src/` file. To convert a unit: change its subsegment
  from `asm` to `c` in `superbowling.yaml`, run `make split`, then `make gate`.
  - Overlay units (pattern set by gacode): subsegment `[<rom>, c, <seg>/<unit>]` gives `src/<seg>/<unit>.c` and
    `asm/nonmatchings/<seg>/<unit>/`; headers in `include/<seg>/`. Symbols keep the `_<seg>` suffix
    (`symbol_name_format: $VRAM_$SEG`; the overlays share VRAM, `exclusive_ram_id: ovl`). The segment's
    `bss_size` and `{ type: bss, vram: ... }` subsegment scope its bss symbols to it. Each unit's `.rodata`
    (doubles, jump tables) is a `[<rom>, .rodata, <seg>/<unit>]` subsegment after all units' `.data`; splat moves
    rodata used by one function into that function's asm, and the rest needs the `INCLUDE_RODATA` lines splat
    writes into a fresh C file (regenerate one in a scratch copy to see where they go).
  - Data in C (done for mgcode, rscode, pvcode, cacode, opcode, gscode, gacode, part of main): replace the asm `[<rom>, data]` subsegment with
    `[<rom>, .data, <seg>/<unit>]` and the bss one with `{ type: .bss, vram: ..., name: <seg>/<unit> }`, set `bss_contains_common: True` on the
    segment (gcc's uninitialised globals are commons; splat then links `<unit>.o(.bss COMMON .scommon)`; the flag
    must be on the code segment, which overrides its subsegments'), and define every datum in the unit in address
    order with its real type and initialiser (functions by name, cast to the member's function-pointer typedef
    where a handler's prototype differs). `rm -rf asm && make split && make gate` verifies it. Bss is accounted
    for by real objects only: a range nothing references is never filled with invented storage. Commons are
    placed in the linker's hash order, not in source order; when bss objects sit in definition order with their own
    alignment (rscode: the `u8[4]` `rs_ranks` right after a `u16`, at 0x801142A2), they are file-scope `static`s,
    which the assembler allocates in `.bss` in that order (no `bss_contains_common` then). KMC as pads `.bss` to 16
    bytes and the unit's commons follow it (cacode's `ca_bmpBuffer` at 0x8013E880, 4 bytes after its statics).
    Several units (opcode): the linker lays out each unit's `.data` and `.bss` in link order, each padded to 16, so
    assign every datum to the unit whose code references it, check that the owners are contiguous in link order and
    that the 16-byte gaps fall at their boundaries, then give each unit with data its own subsegment
    (`[<rom>, .data, <seg>/<unit>]`, `{ type: .bss, vram: ..., name: <seg>/<unit> }`; a unit with none gets none,
    as opcode's 6BE70 has no `.data`). A datum only one unit uses is a file-scope `static` there; the header keeps
    the types. gcc word-aligns every array and struct (opcode's `op_pakImages` at 0x801141CC after a `u16[0x85]`),
    so a 2-byte gap after an array is alignment; an unreferenced run with real contents inside a unit's range is
    a datum of that unit, defined under a placeholder name (opcode's two glyph strings at 0x80114254).
    Data whose users interleave (gacode) goes into a data-only unit (`src/<seg>/data.c`, no functions, only a
    `[<rom>, .data, <seg>/data]` subsegment), placed in link order where its run starts.
    main: the same, per unit, with an asm `[<rom>, data]` blob for each run not yet in C. A unit with `NO_INCLUDE_ASM` defines its
    data after its functions when a definition before the first one would drop that function's leading hazard nop (1B1A0);
    166B0's command table follows the handlers it names.
- **Headers:** `include/common.h` (types, `INCLUDE_ASM`), `include/structs.h`, `include/variables.h`,
  `include/functions.h`. Each type and global is declared once there (R1, R2).
- **libultra:** 2.0K `libgultra_rom`, linked from source. `lib/ultralib` is decompals' ultralib (git submodule
  pinned at e24c836; `git submodule update --init`). `make` builds it into
  `build/ultralib/K/libgultra_rom/libgultra_rom.a` (TARGET=libgultra_rom VERSION=K COMPARE=0, `tools/gcc_2.7.2`;
  `MDEBUG_FILES=` lets KMC as assemble the unused IDO-only `monutil.s`).
  In `superbowling.yaml`, splat `lib` subsegments place each member's `.text`, `.data`, `.rodata` and `.bss` at its
  retail address; the archive goes on the `ld` command line, so the link pulls members by symbol as the original
  did, and exactly the 161 placed members are pulled (156 with code, plus data-only `vitbl`, `vimodentsclan1`,
  `vimodepallan1`, `vimodempallan1`, `libm_vals`). Placements come from the ROM: each member's relocated
  references give its section addresses, and data-only members were found by content; the gate verifies them.
  - `symbol_addrs.txt` names every member symbol with `defined:True` (so splat emits no absolute copy and asm
    references bind to the member's definition), including `parameters.o`'s absolute boot globals at 0x80000300.
  - Commons (tentative definitions such as `__osPiAccessQueue`) were allocated by the original linker among the
    game's bss; `symbol_addrs.txt` names them, so the bss asm defines their storage at the referenced addresses.
    `__osEepromTimer` (`controller.o`) is never referenced, so its address is unknown and `ld` drops it.
  - Ambiguous matches, resolved by callee or data: `piacs` at 0x80043B40 and `siacs` at 0x8004B1B0;
    `osViGetCurrentFramebuffer` 0x8004DFE0 (reads `__osViCurr`); `__osSiRawReadIo`/`__osSiRawWriteIo` (call
    `__osSiDeviceBusy`). `sprite` and `spriteex` (0x8004B2D0) compile to identical bytes and relocations, so the
    ROM cannot decide; `sprite` is linked.
  - After `ldiv` (0x80055660) comes KMC's libkmc, not ultralib: `src/libkmc/` (C runtime, libm and the SVID
    `matherr` hook, ROM 0x30C20-0x320F8; `.data` 0x3A4F0-0x3A7F0, `.rodata` 0x3C1E0-0x3C2A0, `rand`'s `.bss` at
    0x80066B20; declarations in `include/libc.h`), then the SDK's `rspboot` microcode (a `bin` subsegment,
    0x32100-0x321D0). libkmc's `llmath.c` (`__divdi3`, `__moddi3`, `__udivdi3`, `__umoddi3`, `__fixdfdi`,
    `__floatdidf` and three internal division helpers) is hand-written assembly, ORIGINAL-ASM in
    `config/rules.txt`: GCC 2.7.2's `libgcc2.c` was checked and does not apply (no `__udivmoddi4` structure,
    non-ABI register conventions), so it is not adopted. `strcat` calls the game's own `strcpy` (0x80029578).
  - objdiff reports each linked member as a `libultra/<dir>/<file>` unit (`tools/objdiff_config.py`).
  - SDK declarations come from one path: `include/libultra.h` wraps ultralib's `PR/` headers (`os.h`, `gu.h`,
    `libaudio.h`, ...). ultralib ships no n_audio header, so `include/n_libaudio.h` (project header) declares only the
    n_audio calls and `N_ALVoice` the game's code uses, reconstructed from the ROM. Nintendo's SDK sources and headers
    (proprietary) are never copied or used as a model; n_audio code stays asm until it is reconstructed clean-room.
  Symbols outside every splat segment live in `undefined_syms.txt`. Retail ROM: `rom.z64`, SHA-1
  `56937a9e9536af55ebed06480413265848c4a04a`, F3DEX 2.08, libultra 2.0K.
- **ROM map** (from the game's `BW64FAT` file table at ROM 0x3C2A0, which main loads to 0x8010A080 at boot and
  `FAT_GetFileSize` searches by 8.3 name):

  | segment | ROM | VRAM | notes |
  |---|---|---|---|
  | main | 0x1060-0x3C2A0 | 0x80025C60 | 604 functions; BSS to 0x8010A080. No `SUBALIGN` (`subalign: null`): each object's sections keep their own alignment, as in the original link (KMC as gives `.text`/`.data`/`.bss` 2**4 and pads `.text` to 16; `.rodata` takes its contents' alignment, so 2DF0's `.rodata` follows 1060's at 0x80060388). One link order for every section: game units, libultra members, libkmc, rspboot, then the four microcode objects (gspF3DEX2_Rej, gspF3DEX2, gspL3DEX2, aspMain). So the microcode's `.text` (0x321D0-0x364E0, a `data` blob) follows rspboot's, and the `.data` section opens at 0x364E0 with the game's: the units' `.data` 0x364E0-0x38520, then libultra's, libkmc's, and the microcode's `.data` (0x3A7F0-0x3B6C0, a `data` blob). Game `.data` all in C: first the data-only unit `src/data.c` (0x364E0-0x37590, the shared globals, whose owners interleave), then per unit 7250, BEF0 (an unreferenced `u8[2][24]`, the first candidate in link order after 7250's padded `.data`), C650, 111A0, 12260, 12C70, 13230, 13490, 166B0, 19F50, 1ADA0, 1B1A0, 1B2D0, 1B890, 1CB60 (three unreferenced `f32`s, 0, -10.0, 0: the place of the initialised statics of ultralib's `reverb.c`), 1D550 (`snd_globals`, `snd_synth`). An unreferenced zero run that alignment can't explain is its own placeholder datum (user ruling: gcc 2.7.2 puts `x = 0` in `.data`). Bss: 0x80060EA0-0x800645C0 holds the game units' `.bss` (file-scope statics) in link order (1060, 2DF0, 3D50, 4890, 4DC0, 51F0, 7250, 111A0, 12260, 166B0, 1A7A0, 1ADA0, 1B060, 1B1A0), before libultra's `.bss`: in C as file-scope statics for 3D50, 4890, 4DC0, 7250, 12260, 166B0, 1ADA0, 1B060 and 1B1A0, still asm for 1060 and 51F0 (unreferenced words), 2DF0 (`cont_siEvent`'s size is unknown), 111A0 (an unreferenced 8 bytes before `gridEd_marked`) and 1A7A0 (an unreferenced word); 0x80066B30-0x8010A080, after libkmc `rand`'s, is `COMMON`: every unit's uninitialised globals, the game's and libultra's (`__osPiAccessQueue`, `__osContPifRam`), which the linker allocates after all `.bss` sections, still asm. Per-unit `.rodata` subsegments after the microcode's `.data`: 1060 (0x3B6C0), 2DF0 (0x3B788), 53B0, ... |
  | fat | 0x3C2A0-0x3C470 | 0x8010A080 | `BW64FAT` table: {name ptr, ROM start, ROM end, 0} |
  | knstatic | 0x3C470-0x3C570 | 0x8010A250 | `KNSTATICSYS` |
  | gacode | 0x3C570-0x56FF0 | 0x80112350 | game overlay (`GACODE  SYS`): C units 3C570, 3E350, 474E0, 4C560 and the data-only unit `data`; .data 0x4D820 in C (3C570's state to 0x4D8B0, then `data`'s interleaved tables, then 474E0 at 0x56B20 and 4C560 at 0x56B40); .rodata 0x56C20 (per unit, confirmed by the gate); bss 0x8012CDD0-0x8012F350 still asm (ends with the second lane) |
  | rscode | 0x56FF0-0x58F40 | 0x80112350 | results overlay (`RSCODE  SYS`): C unit 56FF0; .data 0x58EF0; .rodata 0x58F30 (one auto array initialiser); bss 0x801142A0, 0x20 (the end 0x801142C0 is what main's loader `Game_Thread` clears to) |
  | gscode | 0x58F40-0x67370 | 0x80112350 | game-select overlay (`GSCODE  SYS`): 12 C units 58F40-61CA0; .data 0x62130, per unit and defined in C (`gs_classes`, first, is 58F40's by position; each unit's .data padded to 16); .rodata 0x67260 (per unit: 58F40, 59490, 5AD50, 5E2B0, 605D0, 61640); bss 0x80120780, 0x2F0 (the end 0x80120A70 is what main's loader `Game_Thread` clears to): 58F40's, 605D0's and 61CA0's in C, 5AD50/5EC00's (0x80120790) and 61640's (0x80120A20) in asm, since KMC's assembler put objects of 16 bytes or more at a multiple of 8 and 0x80120799-0x801207B1 is unreferenced |
  | cacode | 0x67370-0x6A650 | 0x80112350 | overlay (`CACODE  SYS`): C unit 67370 (text to 0x6A37C; no alignment padding between its six functions, and its rodata follows their order); .data 0x6A380; .rodata 0x6A570 (file names, debug format strings, doubles); bss 0x80115630, 0x29260 (the motion tables saved as "motion.dat"; the end 0x8013E890 is what main's loader `Game_Thread` clears to) |
  | pvcode | 0x6A650-0x6BA90 | 0x80112350 | overlay (`PVCODE  SYS`): C units 6A650, 6A940 (the scene's two empty class handlers; text to 0x6A950); .data 0x6A950; .rodata 0x6BA40 (a file name and debug format strings); no bss (the file end 0x80113790 is what main's loader `Game_Thread` clears to) |
  | mgcode | 0x6BA90-0x6BE70 | 0x80112350 | overlay (`MGCODE  SYS`): C unit 6BA90 (text to 0x6BC88); .data 0x6BC90; .rodata 0x6BE30 (debug format strings); bss 0x80112730, 0x1000 (a 21 x 21 maze grid, the rest unreferenced; the end 0x80113730 is what main's loader `Game_Thread` clears to) |
  | opcode | 0x6BE70-0x6DE70 | 0x80112350 | overlay (`OPCODE  SYS`): C units 6BE70, 6C470, 6CC50 (text to 0x6DBC8); .data 0x6DBD0 (6C470), 0x6DBE0 (6CC50); .rodata 0x6DDD0 (6C470's jump table); bss 0x80114350 (6BE70), 0x80114360 (6C470), 0x80114370 (6CC50), 0x2B0 in all (the end 0x80114600 is what main's loader `Game_Thread` clears to) |
  | idx, str, bgmptr, bgmwbk, septr, sewbk, bfx | 0x6DE70-0x656CD0 | - | asset files (bin) |
- **Toolchain:** compiler and linker version and build, flags, per-unit flag overrides.
- **Annotations:** the address marker format (for example `// FUNCTION: <MODULE> 0x...`), the stub marker and
  the data marker.
- **Reference sources:** surviving original source, SDKs, sibling games or engines, and how far each is trusted.
- **Policies:** whether running the game or using a decompiler is allowed, where generated output goes, and
  the commit message style.

## Objective and scores

- The goal is an exact match for every function: identical instructions, constants and *ordered relocation
  targets*. Fuzzy and aggregate percentages are navigation only.
- If the project tracks per-function history, keep three numbers per function:
  - **current**: the score at the latest build;
  - **best**: the best score of the function's current source. It resets when the source changes;
  - **historical**: the best score ever, across all source revisions.

  `historical > best` means an earlier source matched better: a lost match to recover from version control
  before inventing anything new.
- A current-score dip in an *unchanged* function, caused by codegen shifting elsewhere in its translation unit,
  is not a regression.
- Masked bytes are not enough. A matching body that calls the wrong function, or names the wrong global, is a
  defect.

## Workflow

1. **Pick work.** Choose the lowest-scoring or never-matched functions, lost matches, or a unit close to complete.
   Check existing notes and verdicts (source comments, ledgers, history) first.
2. **Classify the gap** (`wall-identifier`): referent, then inline/call set, then control flow, then
   register/schedule. Fix the earliest class that still differs.
3. **Reconstruct** with `matcher`; recover hidden helpers with `helper-recovery`. Use `permute` only for a
   diagnosed register/schedule residue.
4. **Iterate** with `match <unit>`. Run `build` when a change spans units, for example a shared-header edit.
5. **Gate** before committing or merging. The gates must stay green.
6. **Stop.** Mark a complete body whose remaining difference is bounded by evidence, using the project's
   early-stop or stub marker, with a one-line reason.

## The byte gate

A function is done when **the whole binary, linked with that function compiled from source, still hashes to the
retail binary's hash**. Nothing else counts: not "looks equivalent", not a passing test, not a local diff of zero,
not a closeness score, not an agent's verdict. Every other signal is a filter feeding this gate. That is what makes
automation safe: a weak model or a buggy tool can waste time, but a wrong answer cannot land.

- **The gate has two states**, identical or failed; there is no third. It needs:
  - one hash contract per binary;
  - a build that links matched objects together with fallback bytes from the original for everything not yet
    matched;
  - a check that exits non-zero on any mismatch.

  Unmatched or non-matching source never enters the gated build.
- **Build it first.** A link made entirely from the original's bytes must reproduce the hash before progress is
  measured against it.
- **What the gate is blind to, and the rule that covers each blind spot:**

| blind to | rule |
|---|---|
| **coverage**: the gate is green at 0%, because the fallback supplies the original bytes | report how much of the image came from source, counted from the link's own record of where each byte came from; keep a second, independent check (per-function match) and treat any disagreement between the two as a bug |
| **which hash** | the gate verifies that its contract belongs to the binary it builds, and that the configured original still has that hash |
| **whether the build ran**: a failed link leaves the old binary in place | delete the output first, and judge from the linker's and the build's exit codes, never from the file |
| **incremental state** | gate only from a clean rebuild into a fresh directory |
| **whether the body is source**: pasted assembly (`__asm`, emitted bytes) matches trivially | inline-asm bodies are "verbatim", never counted as matched source; audit them (functions that were assembly in the original are `ORIGINAL-ASM`, listed apart) |
| **whether the source is honest**: fake code (dummy locals, `volatile` carriers, offset casts) matches too | the rule checker (`rules`) runs over every matched function; [match] findings make it `MATCH-PENDING` |
| **what was committed**: a gate that substitutes drafts can destroy uncommitted work | commit matched work the moment it exists, and never gate on a dirty tree (other than tool-database churn) |

- **"Banked" is a printed line, not an intention.** Write "matched" or "banked" only from the gate's printed success
  line. A helper must refuse an empty work list. Count from the source, never from a ledger.
- **A red gate is a hash, not a diagnosis.** Turn it into a named symbol: attribute the differing bytes to the
  function and unit that hold them, or substitute the one draft into a copy of its unit and read the compiler's first
  conflict or redefinition error.
- **Batch, then bisect.** One clean rebuild verifies a batch of drafts. On red, bisect, or drop the units the byte
  attribution names, so that one bad draft cannot sink the rest. When parallel drafters declare a shared callee
  differently, the failure is the clash, not either draft: normalise the declarations, gate one at a time, and re-gate.
- **A slow gate is a bug.** Measure it before you fear it; run builds in parallel, and give the expensive binary its
  own lane rather than excluding it.

## Code rules

The full catalogue is the user-level `decomp-code-rules` skill (`references/rules.md`). Two laws apply always:

1. **The bytes win.** A change made for style must not change what the compiler emits. When a rule and the bytes
   disagree, keep the bytes and write `// RULE-EXCEPTION: <rule>, <reason the bytes give>` at the code.
2. **Identical bytes alone are not a match.** A function counts as matched only when its bytes match *and* it
   passes every **[match]** rule:
   - R1: each type declared once, in a shared header;
   - R2: each global declared once, as `extern` in a header, and defined in its owning unit;
   - R4: no fixed-offset casts, and no offset arithmetic on `this`;
   - R13: real vtables: every slot in the binary's vtable declared, none added (a slot of unknown purpose keeps
     a placeholder name);
   - R17: no fake code (probes, `volatile` carriers, dummy locals, steering blocks);
   - R18: no assembly bodies.

   A function that fails is `MATCH-PENDING`: keep its source, report it, and don't count it. A function the
   evidence shows was originally assembly is `ORIGINAL-ASM`, listed apart. Naming (R7) is **not** required for a
   match; placeholder names are open work.

Progress reports two numbers: **matched under the rules** (the headline) and **bytes matched**.

Project profile (fill in):

| item | this project |
|---|---|
| compiler dialect: what it accepts and rejects (R22) | GNU C (gcc 2.7.2): C89 plus GNU extensions; `//` comments accepted |
| naming-style table, one row per module if styles differ (R8); without evidence, start from `style-defaults.md` | Table 1 (Quake III C style) from `decomp-code-rules/references/style-defaults.md`, all modules (user ruling); libultra keeps SDK names; libkmc keeps ANSI C/SVID names |
| layout style: braces, indent, line limit, function and section banners (R23, R32, R33) | Table 1 layout: K&R braces with spaces inside parentheses, `/* ===== Name ===== */` banners |
| placeholder patterns (R7) | `func_8XXXXXXX`, `D_8XXXXXXX`, `jtbl_`, `L8XXXXXXX` (splat defaults); overlay symbols add `_<segment>` |
| evidence sources for names: strings, symbols, RTTI, sibling source (R7) | ROM strings (e.g. the `BW64FAT` file names); libultra and n_audio SDK names (matched against ultralib, or from the SDK code a function reproduces); no symbols. Provenance: `config/names.txt`, one line per recovered name (name, address or owner, `evidence:` or `invented:` with the reason). Module prefixes: `Snd_` functions and `snd_` globals for the sound driver (`src/166B0.c`) |
| does the binary record source lines? (if yes, layout is [checked]: R23) | No debug info; check for `__FILE__`/`__LINE__` assert strings before relying on that |
| parking mechanism for unfinished functions (exempt from R37) | `INCLUDE_ASM("asm/nonmatchings/<unit>", <func>);` (`include/include_asm.h`). A best draft may be kept as `#ifdef NON_MATCHING` / `// NON_MATCHING: <what still differs>` / draft / `#else` / the `INCLUDE_ASM` line / `#endif`; the build never defines `NON_MATCHING`, and `tools/rules.py` skips that branch |
| rules this project tightens or relaxes, with the reason | |

## Matching rules that are easy to get wrong

- **Levers are disposable experiments.** Never keep probes, unused declarations, fake locals, `volatile`
  carriers, padding or distorted source to move a score. A trick that closes a gap is a clue to the real source
  (`decomp-code-rules`, Section F), not the answer.
- **A dip is not a rejection.** If a change moves codegen toward retail's shape, keep it and compose the next
  lever on top. The gate decides only what is committed.
- **Prefer original spellings.** An inline function or macro is more likely the original spelling than a
  hand-expanded body; prefer it as the base unless the evidence says otherwise.
- **Trust surviving source.** Surviving original or lineage source is presumed authentic; record the decision
  to adopt it where the project keeps such decisions.
- **Bank exacts reached by experiment.** If unchanged source reaches exact under a disposable translation-unit
  experiment, record the exact state, then remove the experiment.
- **Keep every exact match, and count it honestly.** Keep a 100% match even when its source breaks a rule.
  - **A style rule the bytes defeat:** write a `RULE-EXCEPTION` at the code. The function counts.
  - **A [match] rule:** the function is `MATCH-PENDING`. Keep its source as the best-known state, and list it
    as open work.

  Record either case (function, rule, deviation) where the project keeps exceptions, and admit it through the
  gate's allow list.

## Source rules

- **One class, one definition**, in a shared header. No unit-local copies, layout views or placeholder shells.
- **Each function and global lives in its evidence-backed owner** unit and header. No scattered `extern`s and no
  macro aliases onto address names.
- **Access goes through typed members.** No raw offset casts, offset macros or offset arithmetic on `this`. Use named casts
  only at real boundaries. A cast is usually a symptom: retype the member until it disappears.
- **Unclear identity: chase the evidence** (callers, storage, callees, mangling, vptr stores, RTTI, offsets). If
  it is still unclear, leave an identity-TODO marker. Never fabricate.
- **Names are semantic.** No address-derived names, compiler ordinals or `local_10`-style names in finished
  code. Take names from evidence first: error strings that start with a function name, console-variable
  strings, assert text, symbols. A name you invent is marked as invented wherever the project records
  provenance. Follow the module's own naming style.
- **Readable by default.** Recovered source should read like the original team's code:
  - one local per meaning (R30);
  - named constants and flag groups (R11, R29);
  - documented structs (R27);
  - a header comment per function (R33);
  - the original's bugs kept and labelled, not fixed (R36).
- **Use the era's real headers** (SDK, MFC, DirectX) instead of hand-rolled declarations. Mirror the original
  precompiled-header or prelude structure when the evidence shows one.
- **Typed enums for proven numeric domains.** Remember that retyping a parameter changes its mangled name.
- **Aggregates are proven from whole-object use** (`RECT`, `POINT`, vectors). Do not split one object into
  overlapping globals, and never model an interior address as separate storage: refine the owner.
- **Vtables are recovered mechanically:**
  - one vtable per polymorphic base;
  - inherited slots are not redeclared;
  - overrides are marked (a comment, or a repeated `virtual`, in C++98);
  - new slots are `virtual`;
  - there are never dummy virtuals: a padded vtable can truncate the real one and crash. A real slot of unknown
    purpose keeps a placeholder name.
- **No fake code**, storage, labels, aliases or padding to improve a score or the final layout.

## Data

- A datum gets a real definition with its address annotation. Compiler-generated data (string pools, FP
  constants, dynamic initialisers) is recorded as such, not given an invented owner.
- Aggregate data percentages do not prove that `.data` and `.bss` are correct. The compiler and linker choose
  layout; a field read through a separately declared global gets its own storage, so a link of the source alone
  can behave differently even when every function matches.

## Tests and verification

- Matching work runs no test suites: comparing against retail is the verification, and the gate runs before
  commit or merge.
- Tooling changes run the touched tool's own tests.
- Do not add tests that re-check a function's bytes against retail; the build already does that.
- Claim "matched" or "banked" only from the tool's own printed result (see "The byte gate"), never from
  memory or a ledger.

## Repository hygiene

- Tool inputs go in `config/`, generated output in an ignored `build/`, history in version control. Docs describe
  current usage and contracts only: no diaries, score snapshots or campaign logs.
- Source comments explain the code (`decomp-code-rules` R21):
  - markers;
  - ABI or codegen constraints;
  - explanations of unsafe seams;
  - `RULE-EXCEPTION`s;
  - function header banners and section banners in the project's style (R32, R33);
  - why a block exists;
  - the original's quirks (R36).

  No history, scores, dates or campaign notes.
- Preserve concurrent changes: stage only your own unit of work, and never commit build state. Commit messages
  name the change (`match: reconstruct CThing::Method`, `tools: verify relocation targets`).
