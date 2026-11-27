# Building

The build splits your copy of the retail ROM into assembly and binary assets, compiles the reconstructed C with
the original compiler, links everything at the retail addresses and checks that the result is byte-identical to
the ROM. Nothing from the game is in the repository: you supply the ROM.

## Prerequisites

Linux (or WSL) with `make`, `git`, `curl` and Python 3 (CI uses 3.12).

| what | where it goes | notes |
|---|---|---|
| retail ROM, Super Bowling 64 (USA, `NBWE`) | `rom.z64` | big-endian (`.z64`) byte order; SHA-1 `56937a9e9536af55ebed06480413265848c4a04a` |
| KMC GCC 2.7.2 and KMC binutils 2.6 | `tools/gcc_2.7.2/` | decompals builds of the N64 SDK compiler and assembler (see below) |
| MIPS binutils (`ld`, `as`, `objcopy`, `objdump`) | on `PATH` | `mips64-linux-gnu-*` by default; Fedora: `binutils-mips64-linux-gnu` |
| splat and its disassembler | Python packages | `splat64[mips]`; CI pins `splat64==0.41.0 spimdisasm==1.41.0 rabbitizer==1.16.2 n64img==0.3.3 crunch64==0.6.2 pygfxd==1.0.5` |
| ultralib (libultra 2.0K source) | `lib/ultralib` | git submodule |
| decomp-permuter (optional) | `tools/decomp-permuter` | git submodule; only for `tools/permute.sh` |
| objdiff-cli 3.8.2 (optional) | `tools/objdiff/objdiff-cli` | needed by `make report` and the TUI diff |

Fetch the compiler and assembler:

```sh
mkdir -p tools/gcc_2.7.2
curl -L https://github.com/decompals/mips-gcc-2.7.2/releases/download/v0.1/gcc-2.7.2-linux.tar.gz | tar xz -C tools/gcc_2.7.2
curl -L https://github.com/decompals/mips-binutils-2.6/releases/download/v0.3/binutils-2.6-linux.tar.gz | tar xz -C tools/gcc_2.7.2
```

For `make report` and the TUI diff, also fetch objdiff-cli (as CI does):

```sh
mkdir -p tools/objdiff
curl -fsSL -o tools/objdiff/objdiff-cli https://github.com/encounter/objdiff/releases/download/v3.8.2/objdiff-cli-linux-x86_64
chmod +x tools/objdiff/objdiff-cli
```

The C units are compiled by the KMC `gcc` driver, which runs KMC `as` 2.6 on both the compiled code and the
assembly that `INCLUDE_ASM` pulls in; the matching code depends on it (for example, KMC `as` expands `move` to
`addu` where modern GAS uses `or`). Asm-only units, the binary assets and the link use the system binutils.

On distributions that only package `mips-linux-gnu-*` (Ubuntu), pass the prefix to make, as CI does:
`make CROSS=mips-linux-gnu- gate`. `tools/fdiff.py` (behind `make diff`) calls `mips64-linux-gnu-objdump` by name.

## First build

```sh
git submodule update --init
make split      # splat: disassemble rom.z64 into asm/ and assets/ (both ignored)
make gate       # clean rebuild and SHA-1 check; prints "GATE OK: ..." on success
make expected   # copy the green build to expected/, the target that diffs and objdiff compare against
```

`make split` reads `superbowling.yaml` and `symbol_addrs.txt`. It never overwrites an existing file in `src/`.

## Commands

| command | what it does |
|---|---|
| `make` | build `build/superbowling.z64` (incremental) |
| `make gate` | delete `build/`, rebuild everything, compare the ROM's SHA-1 with retail |
| `make expected` | run the gate, then snapshot `build/` as `expected/build/` |
| `make report` | objdiff report (`report.json`), then the progress summary: matched under the rules, and bytes matched |
| `make rules [UNIT=src/<unit>]` | the code-rule checker (`tools/rules.py`); exits non-zero on any [match] finding |
| `make diff UNIT=src/<unit> FUNC=<func>` | side-by-side disassembly of one function, `build/` against `expected/` |
| `make rules-test`, `make rename-test`, `make n64crc-test` | the tests of `tools/rules.py`, `tools/rename_member.py` and `tools/n64crc.py` |
| `make assets-check`, `make assets-test` | extract the ROM's assets, pack them and compare (below); the tool's tests; opt-in |
| `make shift-test [SHIFT=0x40] [AFTER='build/src/1060.o(.text)']` | the shiftability check (below); opt-in, not part of the gate |
| `make mod` | a modified ROM, `build/mod/superbowling.z64`, padded and with its checksum recomputed (below); opt-in, not part of the gate |
| `make clean` | delete `build/` |

`tools/objdiff/objdiff-cli diff -u src/<unit> <func>` gives the same comparison as `make diff` in objdiff's TUI.

## What the checks mean

- **The gate** (`make gate`) is the only test of correctness. It starts from an empty `build/`, so a failed
  compile or link cannot leave an old ROM behind, and it judges by exit codes and the hash. Functions still in
  assembly contribute the retail bytes, so the gate is green whatever the progress; it proves that the C that is
  there compiles to exactly what retail holds, function by function and relocation by relocation.
- **The report** (`make report`) counts progress. "Bytes matched" is objdiff's count of code bytes that come from
  C and match. "Matched under the rules" (the headline) counts functions whose bytes match *and* whose source
  passes the [match] rules of `tools/rules.py`; a function that matches only through fake code is
  `MATCH-PENDING` and is not counted. Hand-written assembly in the original (`ORIGINAL-ASM` in
  `config/rules.txt`) is counted neither way. Game code and libultra are reported apart.
- **The rule checker** (`make rules`) prints `file:line: Rnn [match|advisory] message`. [match] findings fail it;
  advisories (placeholder names, style) never do. See [contributing.md](contributing.md).
- **`make diff`** prints `<func>: identical (N instructions)` when the compiled function matches `expected/`,
  else the two disassemblies side by side with the differing lines marked `|`. Relocations are shown, so a call
  to the wrong function or a reference to the wrong global shows as a difference.

## Assets

`tools/assets.py` turns the ROM's asset files into editable files and packs them back. The extracted tree
`assets_ext/` is ignored by git; it is derived from the retail ROM (`rom.z64`) and is never committed.

```
python3 tools/assets.py extract rom.z64      # writes assets_ext/ and its manifest.json (about 25 s)
python3 tools/assets.py pack                 # rebuilds every FAT file into assets_ext/packed/
python3 tools/assets.py check rom.z64        # packs in memory and compares each file with the ROM's bytes
make assets-check                            # extract, then check; prints ASSETS OK (opt-in, not part of the gate)
make assets-test                             # the tool's synthetic tests (LZSS, PNG, image conversion)
```

`files/` holds each BW64FAT file as it is in the ROM (overlays, `KNSTATICSYS`, the sound banks), the table itself
and the fill after the last file. `str/NNNN.bin` is str member NNNN, decompressed; `str/NNNN.png` is a member the
game draws as an image, as an indexed PNG of the whole padded canvas (the game draws only `width` columns of it;
edit pixels and the palette, keep the canvas size). Pack compresses members again and rebuilds `IDX     SYS` from
their sizes, so an edited member may change size; `pack` writes one file per FAT file to `packed/`. The packed
files are not yet linked into the ROM: the build still takes them from `rom.z64`.

## Shiftability

A modded ROM must still link correctly when code or data changes size, so every reference has to be a symbol the
linker resolves, never a retail address. `make shift-test` checks this statically, without running the game: it
relinks the objects of the finished build into `build/shift/` twice, once unchanged (which must equal the built
ROM) and once with `SHIFT` bytes of padding after the input section `AFTER` (any `build/...o(.section)` line of
`superbowling.ld`, e.g. an overlay unit's `.text`), keeping the relocations (`ld -q`). It then reports:

- **symbol**: a symbol after the insertion point (in its segment's VRAM or a segment linked after it in VRAM, or a
  ROM offset) that did not move by
  `SHIFT`, or one before it that moved: typically an absolute assignment pinning it to its retail address;
- **word**: a 32-bit word of a code or data segment (binary assets are not scanned) that still holds a moved
  address or a moved segment's ROM bound: a raw number where a pointer should be;
- **lui**: a `lui` without a relocation that loads the upper half of a moved address;
- **overlap**: sections other than the overlays that now overlap in VRAM or ROM.

Known findings are listed in `config/shift_known.txt` with their reasons (none at present: a shift in main or in
an overlay prints `SHIFT OK: nothing points at an old address`); the test prints `SHIFT OK` when every finding is
listed and `SHIFT FAIL` (non-zero exit) otherwise. The build helps by linking splat's
`undefined_*_auto.txt` as `PROVIDE` (an address applies only to a symbol no object defines, so C definitions move),
by `reloc_addrs.txt` entries that bind asm references to their owning symbols (interior addresses, the overlays'
bss ends, the BW64FAT table's ROM offsets), and by `undefined_syms.txt` naming rspboot's bounds from its object.

The memory map follows from the link, so main and the overlays can grow:

- `fat` and `knstatic` follow main's bss (`follows_vram` in `superbowling.yaml`), the RDP FIFO is the `rdpfifo`
  segment (`sched_rdpFifo`, 0x8000 bytes of RAM with no ROM contents) after them, and every overlay follows the
  FIFO; overlays still share one slot (`exclusive_ram_id: ovl`).
- The heap and the framebuffers are constants above the link (`HEAP_START`, `HEAP_END`, `SCHED_FRAMEBUFFER( i )` in
  `include/boot.h`; main loads them as `lui`/`ori` constants, so they stay numbers). `undefined_syms.txt` asserts at
  link time that every overlay's bss ends at or below `HEAP_START` (0x80150000) and that the depth buffer
  (`scene_depthBuffer`, 0x80000400) ends below the entry code. Retail leaves 0x11770 bytes of room (cacode's bss
  ends at 0x8013E890); a larger growth fails the link with "an overlay's bss runs into the heap", and needs
  `HEAP_START` moved up (the heap shrinks by as much).
- splat leaves numbers that are not addresses as numbers (`ignore:True` in `symbol_addrs.txt`, for example the
  gSPTextureRectangle words of `Scene_DrawPuffs` and `GridEd_Frame`).

What remains:

1. **`reloc_addrs.txt` lines are tied to ROM offsets** of functions still in asm: a shift does not move them (they
   are applied to the retail ROM by `make split`), but a function's lines must go when it is matched in C.
2. **IPL3 copies only the first 1 MB** (ROM 0x1000-0x101000) to `entry`'s address: main (0x1060-0x3C2A0) must stay
   within it, which it is far from filling.

### Modified ROMs

`make mod` links the same objects as `make`, without the retail pad (`assets/pad.bin`: 0xFF from 0x656CD0 to
8 MB, no data), into `build/mod/`, fills the ROM with 0xFF to 8 MB (to the next power of two if it is larger) and
recomputes the header checksum with `tools/n64crc.py` (the CIC-6102 algorithm over ROM 0x1000-0x101000;
`tools/n64crc.py ROM` checks one, `--fix` rewrites it). Unmodified, `make mod` gives the retail ROM. The normal
build keeps the pad and retail's checksum constant in the header asm, so a grown ROM from `make` is larger than
8 MB and fails the gate.

## Compiler flags

All C is compiled with `-c -G0 -mips3 -mgp32 -mfp32 -O2` (plus `-Wa,-N`, KMC `as`'s N64 alignment of common
symbols). Plain `char` is signed. Exceptions, each set in the `Makefile` from evidence in the retail code:

- `src/libkmc/*.c`, KMC's C runtime, build with `-O`: its code is unscheduled.
- Units `2DF0`, `3AD0`, `4890`, `4DC0`, `53B0`, `gscode/58F40`, `gscode/5EC00` and `opcode/6CC50` build with
  `-O3`: retail inlines earlier functions of the unit at their call sites without `inline` (gcc 2.7.2's `-O3`
  turns on `-finline-functions`).

The libultra archive is built from `lib/ultralib` as `libgultra_rom`, version K (2.0K), with the same compiler,
into `build/ultralib/K/libgultra_rom/libgultra_rom.a`; the link pulls its members by symbol, and splat `lib`
subsegments place each at its retail address.

## Common failures

| symptom | cause and fix |
|---|---|
| `make split` fails, or splat reports a bad checksum | `rom.z64` is missing, byte-swapped (`.v64`/`.n64`) or another revision; check `sha1sum rom.z64` |
| `No rule to make target` for `asm/...`, or a link with missing symbols right after cloning | run `make split` first; `asm/` and `assets/` are generated |
| `lib/ultralib` is empty, or `libgultra_rom.a` fails to build | `git submodule update --init` |
| `tools/gcc_2.7.2/gcc: not found`, or `cc1`/`as` not found | both archives must be extracted directly into `tools/gcc_2.7.2/` |
| `mips64-linux-gnu-ld: not found` | install MIPS binutils, or set `CROSS=mips-linux-gnu-` |
| link errors about undefined or duplicate symbols after pulling | names or segments changed: regenerate the assembly with `rm -rf asm && make split` |
| `make diff` or objdiff compares against nothing | `expected/` is missing or stale: run `make expected` on a green gate |
| the gate prints a `sha1sum` mismatch instead of `GATE OK` | some C compiles to different bytes. Use `make report` or `make diff` on the units you changed to find the function; if a unit no longer compiles, read the compiler's first error |
| a new draft compiles alone but the gate fails | the draft probably moved data or `.rodata` (literals, jump tables) or changed a shared declaration; compare the unit's sections, not just the function |

Never judge a build from the presence of `build/superbowling.z64`: only the gate's printed `GATE OK` line counts.
