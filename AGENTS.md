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
| `build` | compile every unit, compare every annotated function | TODO (needs compiler + binutils) |
| `match <unit>` | compile and compare one unit quickly (the inner loop) | TODO |
| `diff <function>` | side-by-side disassembly: compiled vs retail | TODO (objdiff / asm-differ) |
| `report` | refresh the progress report (objdiff or similar) | TODO |
| `gate` | the checks that must stay green before a commit or merge | TODO: clean rebuild, `sha1sum -c` against `56937a9e9536af55ebed06480413265848c4a04a` |
| `disasm` / `xref` / `class` | retail assembly, callers and callees, vtable and hierarchy evidence | `python3 -m splat split superbowling.yaml` → `asm/` |
| `permute <function>` | the permuter, if there is one | TODO (decomp-permuter) |
| `rules [<unit>]` | the rule checker: [match] findings (`MATCH-PENDING`) and advisory findings (naming, readability) | TODO |

Also record:
- **Toolchain (this project):** KMC GCC 2.7.2 (N64 SDK), user ruling. Evidence: epilogues place `addiu sp` in the
  `jr ra` delay slot throughout, including the libultra region (KMC `libgultra`); splat also detects KMC.
  Optimisation flags not yet determined. Retail ROM: `rom.z64`, SHA-1 `56937a9e9536af55ebed06480413265848c4a04a`,
  F3DEX 2.08, libultra 2.0K. Main segment ROM 0x1060 at VRAM 0x80025C60, BSS ends 0x8010A080; code past ROM
  0x3C2A0 runs at 0x801xxxxx (overlays, boundaries not yet mapped).
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
| naming-style table, one row per module if styles differ (R8); without evidence, start from `style-defaults.md` | Table 1 (Quake III C style) from `decomp-code-rules/references/style-defaults.md`, all modules (user ruling); libultra keeps SDK names |
| layout style: braces, indent, line limit, function and section banners (R23, R32, R33) | Table 1 layout: K&R braces with spaces inside parentheses, `/* ===== Name ===== */` banners |
| placeholder patterns (R7) | `func_8XXXXXXX`, `D_8XXXXXXX`, `jtbl_`, `L8XXXXXXX` (splat defaults) |
| evidence sources for names: strings, symbols, RTTI, sibling source (R7) | ROM strings; libultra 2.0K / libgultra SDK source; no symbols |
| does the binary record source lines? (if yes, layout is [checked]: R23) | No debug info; check for `__FILE__`/`__LINE__` assert strings before relying on that |
| parking mechanism for unfinished functions (exempt from R37) | `INCLUDE_ASM("asm/nonmatchings/<unit>", <func>);` (`include/include_asm.h`) |
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
