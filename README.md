# Super Bowling 64 decompilation

[![Build](https://github.com/CalenCyr/superbowling64-decomp/actions/workflows/build.yml/badge.svg)](https://github.com/CalenCyr/superbowling64-decomp/actions/workflows/build.yml)
[![Code](https://bytematch.dev/CalenCyr/superbowling64-decomp.svg?label=Code&measure=matched_code_percent)](https://bytematch.dev/CalenCyr/superbowling64-decomp)
[![Functions](https://bytematch.dev/CalenCyr/superbowling64-decomp.svg?label=Functions&measure=matched_functions)](https://bytematch.dev/CalenCyr/superbowling64-decomp)

A work-in-progress matching decompilation of **Super Bowling 64** (Nintendo 64, USA, `NBWE`). The goal is C source
that, built with the original compiler, reproduces the retail ROM byte for byte.

This repository contains no game code, assets or ROM. You need your own copy of the game.

## Status

| | matched under the rules | bytes matched |
|---|---|---|
| all code | 901 / 905 functions | 328,072 / 335,856 bytes (97.7%) |
| game | 572 / 576 functions | 252,956 / 260,740 bytes (97.0%) |
| libultra | 329 / 329 functions | 75,116 / 75,116 bytes (100%) |

"Matched under the rules" counts functions whose bytes match *and* whose source passes the project's code rules
(no fake matches); 10 hand-written assembly helpers are listed separately as original assembly. Run `make report` for current numbers; live progress is on
[ByteMatch](https://bytematch.dev/CalenCyr/superbowling64-decomp), updated by CI on every push to `main`.

## Documentation

- [Building](docs/building.md): prerequisites, `make gate`, `make report`, what each check means, common failures.
- [Architecture](docs/architecture.md): how the game works: boot and threads, overlays and scenes, graphics,
  sound, input, save data and files.
- [Contributing](docs/contributing.md): the matching workflow, the byte gate, the code rules and the project's
  rulings, naming and provenance.
- [Native port design](docs/porting.md): how the game uses the N64 hardware, measured, and a plan for a native
  PC port.
- [AGENTS.md](AGENTS.md): the full contract for matching work, including the ROM map and the toolchain evidence.

## What's known about the ROM

- **Compiler:** KMC GCC 2.7.2 from the N64 SDK, flags `-O2 -G0 -mips3 -mgp32 -mfp32` (a few units use `-O` or
  `-O3`; see [building.md](docs/building.md)).
- **Library:** libultra 2.0K (`libgultra_rom`), built from source: the
  [ultralib](https://github.com/decompals/ultralib) submodule in `lib/ultralib`, compiled with the same compiler and
  linked at the retail addresses. KMC's C runtime (libkmc) follows it.
- **Layout:** a main segment at `0x80025C60`, plus seven code overlays that all load at `0x80112350`. The game finds
  its files through its own `BW64FAT` file table. The full ROM map is in [AGENTS.md](AGENTS.md).

## Quick start

You need Linux (or WSL), Python 3, MIPS binutils, KMC GCC 2.7.2 with binutils 2.6 in `tools/gcc_2.7.2/`, splat
(`pip install splat64[mips]`) and your ROM at `rom.z64` (SHA-1 `56937a9e9536af55ebed06480413265848c4a04a`).
[docs/building.md](docs/building.md) has the details. Then:

```sh
git submodule update --init
make split      # disassemble the ROM into asm/ and assets/
make gate       # clean rebuild; prints "GATE OK" if the output matches the retail SHA-1
make expected   # snapshot the matching build as the diff and objdiff target
```

## Commands

| command | what it does |
|---|---|
| `make` | build `build/superbowling.z64` |
| `make gate` | clean rebuild and SHA-1 check; the test every change must pass |
| `make diff UNIT=src/<unit> FUNC=<func>` | side-by-side disassembly of one function: yours vs retail |
| `make report` | progress: matched functions and bytes, game and libultra |
| `make rules [UNIT=src/<unit>]` | the code-rule checker: fake-match findings and naming/style notes |

## Working on a function

1. Pick an `INCLUDE_ASM("asm/nonmatchings/<unit>", <func>);` line in a `src/` file; its assembly is in
   `asm/nonmatchings/<unit>/<func>.s`.
2. Replace the line with C, then iterate with `make diff UNIT=src/<unit> FUNC=<func>` until it prints `identical`.
3. Run `make rules UNIT=src/<unit>` and `make gate` before committing.

[docs/contributing.md](docs/contributing.md) explains what counts as a match. The campaign follows the Claude Code
skills from [Game-Decompilation-Skill-For-Claude](https://github.com/lemiur/Game-Decompilation-Skill-For-Claude)
(`matcher`, `decomp-code-rules`, `orchestrator` and others); install them from there if you work with Claude Code.

## Layout

```
superbowling.yaml   splat config: segments, overlays, file splits
symbol_addrs.txt    known symbol names and addresses
src/                reconstructed C, one file per original unit (overlays in src/<overlay>/)
include/            shared headers (types, globals, prototypes)
config/             names.txt (name provenance), rules.txt (rule checker allow list)
docs/               building, architecture, contributing
lib/ultralib/       libultra source (submodule)
tools/              diff, progress, objdiff config, the rule checker, permuter and rename scripts
asm/, assets/       generated by `make split` (not committed)
```

## Legal

This project contains only original reconstructed source and tooling. It includes no ROM, game code or assets.
Use it with a copy of the game you own.
