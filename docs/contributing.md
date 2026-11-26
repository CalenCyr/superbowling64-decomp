# Contributing

The aim is C source that a programmer on the original team could have written, and that the original compiler
(KMC GCC 2.7.2, see [building.md](building.md)) turns into exactly the retail bytes. Matching bytes prove the
source is faithful; they don't make it readable, and source that matches through tricks does not count.
[AGENTS.md](../AGENTS.md) is the full contract; this page is the short version.

## Units and parking

- A **unit** is one original object file: `src/<rom offset>.c` for `main`, `src/<overlay>/<rom offset>.c` for an
  overlay (`src/gacode/3E350.c`), `src/libkmc/<name>.c` for KMC's runtime. Units are named by their ROM offset
  until their purpose is clear; their first line says what they hold. Splat subsegments in `superbowling.yaml`
  define the units.
- An unmatched function is **parked** as `INCLUDE_ASM("asm/nonmatchings/<unit>", <func>);`, which assembles the
  retail instructions from `asm/nonmatchings/<unit>/<func>.s` in its place.
- A **draft** that is complete but not yet exact is kept next to its parking line, never in the build:

  ```c
  #ifdef NON_MATCHING
  // NON_MATCHING: <what still differs, and what was tried>
  ... the draft ...
  #else
  INCLUDE_ASM("asm/nonmatchings/<unit>", <func>);
  #endif
  ```

  The build never defines `NON_MATCHING`; the permuter and the rule checker skip that branch as needed.
- Declarations go in headers: types and globals in the unit's header under `include/` (or `include/<overlay>/`),
  each declared once; a global is defined in the unit that owns it. Data-only units (`src/data.c`,
  `src/gacode/data.c`) hold globals whose users interleave in `.data`.

## Matching a function

1. Pick a parked function. Read its assembly, its callers and the notes in the source.
2. Write the C in place of the `INCLUDE_ASM` line, using the shared headers and libultra's own declarations
   (`include/libultra.h`).
3. Iterate with `make diff UNIT=src/<unit> FUNC=<func>` (or objdiff's TUI) until it prints `identical`. Fix
   differences in this order: wrong referents (calls, globals) and missing inlined helpers first, then control
   flow, then register allocation and scheduling.
4. `make rules UNIT=src/<unit>`: no [match] findings.
5. `make gate`: it must print `GATE OK`. Then commit (`match: <unit> <function>`).

If the bytes won't close, keep the draft under `NON_MATCHING` with a note that says what still differs.
`tools/permute.sh` runs decomp-permuter on a function for a diagnosed register or scheduling residue; its output is
a clue to the original spelling, never source to paste.

Some rules of thumb that recur here: an earlier function in the same unit whose body the draft repeats is
probably an `inline` helper (or the unit builds at `-O3`); a field reached as `symbol + constant` through a computed
index is a flat array, not a struct member; statement order matters even where the scheduler reorders.

## The byte gate

`make gate` rebuilds from scratch and compares the whole ROM's SHA-1 with retail. It is the only proof of a
match: not a zero diff in one function, not a score, not a review. A function counts as **matched** only when the
gate is green with it compiled from C *and* it passes the [match] rules below.

## Honest source: the rule checker and fakematches

A **fakematch** is source that produces the right bytes but is not what anyone would have written: dummy locals,
`volatile` added to steer code generation, fixed-offset pointer casts, statements that only nudge the compiler,
pasted assembly. `tools/rules.py` (`make rules`) checks what it can; the rest needs a reader. Its [match] rules:

| rule | requirement |
|---|---|
| R1 | each type declared once, in a shared header; no layout views |
| R2 | each global declared once, `extern` in a header, defined in its owning unit |
| R4 | no fixed-offset casts or offset macros: access goes through typed members |
| R17 | no fake code: no unused or assign-only locals, empty blocks, bare `;`, identity operations, padding members or unexplained `volatile` |
| R18 | no assembly bodies outside `INCLUDE_ASM` |

A function whose bytes match but which fails one of these is `MATCH-PENDING`: kept as the best-known source and
not counted. Advisories (placeholder names R7, naming and layout style R8, a missing header comment R33, ...)
never block a match. The checker can't see a local reused for two meanings (R30), statements that only steer
the compiler, or a wrong referent with identical bytes; reviewers check those by hand.

### Standing rulings

Decisions made for this project, which the checker and reviewers apply:

- **`volatile`** only with evidence, written as `// volatile: <evidence>` on or above the declaration (for
  example, retail reloads the value right after its own store, or reads it with `lbu` and a separate sign
  extension at every use). Each new case is reviewed.
- **Unused locals** are accepted only as genuine leftovers of the original, with the evidence on the declaration:
  `// R36: unused in the original: retail's frame reserves 0x18 bytes no instruction touches`. Locals added just to
  change the frame size are not.
- **Original bugs are kept and labelled** with `R36` (for example, `2DF0`'s Rumble Pak probe of port 1 into port 2's
  `OSPfs`, `RS_RankHighest`'s loop one entry past the end).
- **One local, one meaning (R30).** A local reused for an unrelated second value is not accepted. Accepted, with a
  `// RULE-EXCEPTION: R30, <reason>` comment: one scratch local in the same role in sequence, one local filled
  and queued twice for the same job, a buffer cursor copied from a parameter, a named value computed
  once and used once.
- **Identity operations** that retail really performs (a tuned offset left at zero) are written with an
  `// identity: <evidence>` note.
- **No implicit declarations**: every call has a prototype in a header. A function defined K&R style is
  allowed where the evidence shows callers pass arguments unconverted (`Cache_Release`, declared without a
  parameter list).
- **Exceptions are listed.** A [match] finding the evidence justifies is admitted only through
  `config/rules.txt` (`rule-exception RULE NAME`), with a `RULE-EXCEPTION` comment at the code; for example, unit
  `53B0`'s own non-volatile declaration of `sched_bufferIndex`, and `GA_KeepOnFloor`'s empty branch.
  Functions that were assembly in the original are `original-asm` there (libkmc's `llmath.c`).
- **The bytes win over style.** If a style rule and the bytes disagree, keep the bytes and write
  `// RULE-EXCEPTION: <rule>, <reason>` at the code.

## Names and provenance

- Style: Quake III C (Table 1 of the project's code rules): `Prefix_VerbNoun` functions, `prefix_lowerCamel`
  globals, `lowerCamel` locals, types mostly `lowerCamel_t`, K&R braces with spaces inside parentheses, a
  `================` header comment per function and `/* ===== Name ===== */` section banners. libultra keeps the
  SDK's names, libkmc the ANSI C and SVID names.
- Prefixes by module, for example: `Snd_` / `snd_` the sound driver, `Synth_` the synthesizer, `Audio_` the audio
  thread, `Sound_` the game's sound calls, `Obj_`, `Mtx_`, `Cam_`, `Scene_`, `Draw_`, `Txt_`, `Menu_`, `Score_`,
  `Cache_`, `Str_`, `LZ_`, `FAT_`, `Mem_`, `Host_`, `Cont_`, `Rumble_`, `Save_`, and per overlay `GA_`, `GS_`,
  `RS_`, `OP_`, `CA_`, `PV_`, `MG_` (with `XX_Init` and `XX_Frame` as each overlay's entry points).
- Every recovered name has a line in `config/names.txt`: name, address or owner, and either `evidence:` (a ROM
  string, an SDK symbol) or `invented:` with the reason. Most names are invented; there are no symbols.
- `python3 tools/rename_member.py TYPE OLD NEW --note "invented: ..."` renames a struct member and exactly its
  accesses, driven by the compiler's errors, and adds the provenance line.
- Placeholders (`func_8XXXXXXX`, `D_8XXXXXXX`, `unk_XX`) are open work, not errors.

## Clean-room policy

- Never copy or use as a model Nintendo's SDK sources or headers, or any other proprietary code. The n_audio
  synthesizer (`src/1B2D0.c` to `src/1E140.c`) is reconstructed clean-room from the ROM; ultralib's audio sources
  are not a model for it either, not even in comments. `include/n_libaudio.h` declares only what the game's code
  uses.
- libultra itself comes from decompals' ultralib, an independent reconstruction, built as a library.
- Don't strip copyright notices from anything.

## Source comments and commits

- Comments explain the code: what a function does, why a block exists, ABI and code-generation constraints,
  `RULE-EXCEPTION`s and the original's quirks. No history, scores or dates.
- Stage only your own change, never build state, and gate on a clean tree. Commit messages name the change:
  `match: <unit> <function>`, `park: ...`, `data: ...`, `tools: ...`, `docs: ...`.
