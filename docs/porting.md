# Native port design

This is a design for building Super Bowling 64 as a native program for Windows, Linux and macOS from the
reconstructed source in `src/`. It describes how the game touches the N64's hardware and SDK today, and how a
port should replace each of those interfaces with native APIs at the source level. Nothing here is implemented
yet, and the matching build is not affected by it.

Two kinds of statement appear below:

- **Measured** statements describe the source as it stands, with counts. The call counts come from the
  `R_MIPS_26` relocations of the game's objects in `build/` (C units and the functions still in assembly, without
  libultra and libkmc). The other counts come from greps of `src/` and `include/`. Run `make` first to reproduce them.
- **Recommended** statements are design decisions for the port. They are proposals, not facts about the game.

## Ground rules

- **No ROM data ships with the port.** At startup the port reads the user's own `rom.z64` (SHA-1
  `56937a9e9536af55ebed06480413265848c4a04a`), or the tree that `tools/assets.py extract` derives from it. It
  never ships converted copies of that data.
- **Clean room.** The port does not use Nintendo SDK or other proprietary source, headers, samples or microcode
  implementations, either as a source or as a model. The native renderer and the native audio command
  implementation take their semantics from two sources only: what the game's own code sends, and public
  documentation of the command formats, which the encodings in ultralib's `gbi.h` (already this project's
  dependency) and the project's own `include/n_libaudio.h` express. Copyright notices in files the port reuses are kept.
- **No emulation layer.** The port does not run MIPS code, the RSP or the RDP, and it does not model the N64's
  memory map. It compiles the game's C for the host and replaces the SDK's hardware interfaces with a hardware
  abstraction layer (HAL) shaped by the game's concepts.

## Prerequisite: every function in C

**Measured.** 19 game functions are still `INCLUDE_ASM` (`Game_Thread`, `Scene_AddToDrawList`,
`Scene_DrawPuffs`, `Synth_DecodeSamples`, `Snd_UpdateVolumePan`, `Snd_RelocateWaveBank`, `Txt_NumberToGlyphs`,
`GridEd_Frame`, `Maze_Build`, `Score_DrawFrame`, `Score_DrawRecordTable`, five `GA_*` and three `GS_*`). Every one of
them has a complete `#ifdef NON_MATCHING` draft. libkmc's `llmath.c` is original assembly; it contains the 64-bit
division helpers (`__udivdi3`, called at 4 sites), which the host compiler provides anyway.

**Recommended.** The port builds with `NON_MATCHING` defined, so it compiles the drafts. A draft's comment
names its remaining byte difference. Most of these are register or schedule differences, which don't change what
the function does, but no test proves the drafts equivalent. Treat a draft as a suspect first whenever its
subsystem misbehaves.

## How the game touches the hardware (measured)

### libultra calls by family

| family | call sites | distinct | functions (sites) | where |
|---|---|---|---|---|
| message queues | 76 | 4 | `osRecvMesg` 33, `osSendMesg` 26, `osCreateMesgQueue` 13, `osSetEventMesg` 4 | everywhere; `1060` alone has 80 OS call sites |
| threads | 11 | 3 | `osCreateThread` 5, `osStartThread` 5, `osSetThreadPri` 1 | `1060`, `2DF0`, `1ADA0` |
| interrupts | 2 | 1 | `osSetIntMask` 2 (one critical section) | `1D5D0` |
| VI | 10 | 7 | `osViSetMode` 3, `osViSetSpecialFeatures` 2, `osViSwapBuffer`, `osViSetEvent`, `osViGetCurrent/NextFramebuffer`, `osCreateViManager` | `1060` |
| address translation | 21 | 1 | `osVirtualToPhysical` 21 | display lists and tasks |
| caches | 22 | 4 | `osInvalDCache` 11, `osInvalICache` 7, `osWritebackDCacheAll` 3, `osWritebackDCache` 1 | loaders, overlay loader |
| RSP tasks | 8 | 4 | `osSpTaskLoad` 3, `osSpTaskStartGo` 3, `osSpTaskYield`, `osSpTaskYielded` | `1060` (scheduler), `1A7A0` |
| scheduler client | 2 | 2 | `osScAddClient`, `osScGetCmdQ` | `1A7A0` (the default audio task path) |
| AI | 4 | 4 | `osAiSetFrequency`, `osAiSetNextBuffer`, `osAiGetLength`, `osAiGetStatus` | audio |
| PI / ROM | 15 | 6 | `osEPiStartDma` 5, `__osEPiRawReadIo` 4, `__osEPiRawWriteIo` 2, `osCartRomInit` 2, `osEPiLinkHandle`, `osCreatePiManager` | `3AD0`, `4DC0`, `1A7A0`, `4890`, `47E0` |
| controllers | 7 | 6 | `osContInit`, `osContStartQuery`, `osContGetQuery`, `osContStartReadData` 2, `osContGetReadData`, `osContSetCh` | `2DF0` |
| Controller Pak | 10 | 10 | `osPfsInitPak`, `osPfsFindFile`, `osPfsAllocateFile`, `osPfsReadWriteFile`, `osPfsDeleteFile`, `osPfsFileState`, `osPfsNumFiles`, `osPfsFreeBlocks`, `osPfsIsPlug`, `osPfsRepairId` | `2DF0` |
| Rumble Pak | 3 | 1 | `osMotorInit` 3 (plus the `osMotorStart`/`osMotorStop` macros) | `2DF0` |
| EEPROM / SRAM | 0 | 0 | none (`Sram_GetHandle` sets up a handle that has no caller) | `47E0` |
| time and misc | 10 | 3 | `osGetTime` 7 (profiling only), `osAfterPreNMI` 2, `osGetMemSize` | `1060`, `12C70` |
| gu math | 69 | 20 | `guRandom` 22, `guMtxCatF` 9, `guMtxF2L` 6, `guTranslateF` 4, `guScaleF`/`guRotateRPYF`/`guMtxXFMF`/`guMtxIdentF`/`guMtxIdent` 3 each, `guPerspective`, `guOrtho`, `guLookAt(Reflect)`, `guPosition(F)`, ... | scenes, cameras, objects |
| libm and C | 128 | 7 | `sprintf` 35, `sqrtf` 32, `sinf` 30, `cosf` 24, `coss`/`sins` 4, `atan` 3 (plus single calls of `strlen`, `rand`, `memcpy`, `bzero`) | throughout |
| `al*` (game's audio code) | 40 | 6 | `alHeapDBAlloc` 29, `alLink` 5, `alUnlink` 3, `alHeapInit`, `alFxParam`, `alCopy` | `19F50`-`1E140` |
| `n_alSyn*` | 12 | 8 | the sequencer's calls into the synthesizer | `166B0` |

The `al*` and `n_alSyn*` functions are the project's own clean-room synthesizer (`src/19F50.c` to
`src/1E140.c`), not libultra, and they compile natively unchanged. Only the hardware calls beneath them need
replacing.

### Graphics commands

| what | count |
|---|---|
| GBI macro sites in game C | 290, 49 distinct macros |
| most used | `gDPSetCycleType` 27, `gDPPipeSync` 24, `gDPSetRenderMode` 22, `gDPSetAlphaCompare` 18, `gDPSetTextureLUT` 16, `gDPSetPrimColor` 15, `gSPScisTextureRectangle` 13, `gSPTextureRectangle` 10, `gDPSetCombineLERP` 9 |
| geometry | `gSPVertex` 7, `gSPMatrix` 7, `gSP1Triangle`/`gSP2Triangles` 3, `gSPLine3D`/`gSPLineW3D` 6, `gSPModifyVertex` 1, lights, fog, viewport |
| microcode switches | `gSPLoadUcodeL` 7, among `gspF3DEX2_Rej_fifo`, `gspF3DEX2_fifo` and `gspL3DEX2_fifo` (all F3DEX2 2.08) |
| display lists stored as data | the `KNSTATICSYS` file (256 bytes, 8 objects, 1 absolute pointer), and every model mesh: `Obj_ConvertModel` rewrites 8-byte model commands (end, 1 and 2 triangles, vertex ST) into `Gfx` in place |
| render state from asset data | materials (`fileRefB_t`) carry `gDPSetCombineLERP` arguments, prim and env colours, texture format, wrap and scroll |
| CPU access to framebuffers | `Dbg_ScreenShot` only (development); the depth buffer and the three framebuffers are fixed RAM (`include/boot.h`) |

### Audio commands

**Measured.** The synthesizer builds an RSP audio command list from 15 command macros: `n_aADPCMdec`,
`n_aResample`, `n_aResampleOut`, `n_aEnvMixer`, `n_aPoleFilter`, `n_aLoadBuffer`, `n_aSaveBuffer`, `aMix`,
`aClearBuffer`, `aSetVolume`, `aDMEMMove`, `aLoadADPCM`, `aSetLoop`, `aSaveFrame` and `aInterleaveFrame`; the
ADPCM book and loop state are passed as physical addresses (`K0_TO_PHYS`). All of them are defined in
`include/n_libaudio.h`. The audio thread hands each list to `audio_taskFuncs->runTask` (`audioTaskFuncs_t` = init, waitFrame, runTask). The game
installs its own table (`sound_taskFuncs`, `src/12C70.c`), which sends the task through the main thread's
scheduler. Output is 22050 Hz with 32 voices, and frames are sized in multiples of `AUDIO_FRAME_SAMPLES` (184)
against the AI queue (`Snd_NextFrameSize`).

### Memory, pointers and data

| item | measurement |
|---|---|
| fixed RAM addresses | `RAM_END` 0x80400000, `HEAP_START` 0x80150000, `SCHED_FRAMEBUFFER( i )`, `HEAP_END`, `scene_depthBuffer` 0x80000400 (`include/boot.h`); `main` clears RAM from `boot_staticFile` to `RAM_END` with `memset` |
| other numeric addresses in C | none: `make shift-test` reports `SHIFT OK`, so every other reference is a symbol |
| ROM addresses | linker symbols `*_ROM_START`/`*_ROM_END` (10 uses); file table entries hold ROM start and end; wave tables hold ROM sample addresses (`Snd_RelocateWaveBank` adds `romBase` unless the top byte is 0xFF) |
| overlays | 7 overlays at one address; 247 overlay functions; no duplicate global symbol names across all game objects; `Game_Thread` calls the overlay's entry at the slot address |
| pointers stored over file data | `FILE_RELOC` (13 sites, `Cache_GetFile`) turns 32-bit offsets into pointers in place; `Snd_RelocateTable` (7 calls) does the same for sequences and wave banks; 14 pointer fields in `include/filecache.h`, 45 in `include/structs.h` and 34 in `include/n_libaudio.h` |
| pointer-to-integer casts | 32 casts of a pointer to `u32`/`s32` in game C (for example `s32 base = (s32)data` in `Snd_RelocateTable`) |
| `Gfx` words holding addresses | every `gSPVertex`, `gSPMatrix`, `gSPDisplayList`, `gDPSetTextureImage`, `gDPSetColorImage`/`DepthImage` puts a 32-bit address into `w1` |
| big-endian data | all assets: `IDX     SYS` (3 words per file), images (header words, RGBA5551 palettes), meshes (`Vtx` loaded as is), materials, animations and camera paths (`f32`), sequences and wave banks (word tables, ADPCM books), `BFX     SYS`, and `BW64FAT` |
| endian-safe reads | byte-wise assembly such as `Obj_ConvertModel`'s `( cmd[2] << 24 ) + ...` |
| integer model | `s32`/`u32` are `int` (`include/common.h`); `long` appears 5 times; no bitfields in headers |
| floating point | 250 `f32` declarations or uses, 8 `f64`; `-mfp32` single precision; fixed-point `Mtx` through `guMtxF2L` (6) |
| original quirks kept in the source | 11 `R36` notes (for example the Controller Pak probe of port 1 into port 2's `OSPfs`) |

### Timing

| item | measurement |
|---|---|
| retrace | VI at 60 Hz (NTSC; an MPAL mode is set when `osTvType` asks) |
| game frame | the scheduler shows a new framebuffer at most every second retrace: up to 30 frames a second |
| game logic | counted in frames (for example, the title starts a demo after 450 frames); `Game_Thread` runs one `XX_Frame` per drawn frame |
| audio | the audio thread is woken every other retrace; frame sizes adapt to `osAiGetLength` |
| input | `Cont_Thread` reads the pads each retrace |
| clocks | `osGetTime` only measures CPU, RSP and RDP time for the debug scenes |

## Architecture of the port (recommended)

```
src/ (game C, unchanged where possible)
  |  calls through a port header set: include/port/*.h, selected by -DTARGET_NATIVE
  v
HAL: hal_os (threads, queues), hal_gfx (renderer), hal_audio, hal_input, hal_save, hal_rom
  v
platform backend: SDL3 (window, input, rumble, audio device), OpenGL 3.3 / Vulkan / Metal through one renderer
interface, host filesystem
```

`TARGET_NATIVE` excludes N64-only code: `INCLUDE_ASM`, libultra's archive, the `rspboot` and F3DEX2 binaries,
the PI host link (`src/4890.c`) and the boot code in `asm/`. The HAL declares the game-facing subset of the libultra
names it replaces (`OSMesgQueue`, `OSContPad`, `osRecvMesg`, ...), using the same names so that call sites stay
untouched where the semantics carry over. It declares only what the measured tables above use.

### Threads and the scheduler

**Recommended: a cooperative single OS thread.** Each libultra thread (`Boot_Thread`, `Main_Thread`, `Cont_Thread`,
`Game_Thread`, `Audio_Thread`) becomes a coroutine (fibers on Windows, `ucontext` or a small assembly switch
elsewhere). `osRecvMesg` with `OS_MESG_BLOCK` yields while its queue is empty. `osSendMesg` wakes the waiting thread, and the
highest-priority ready coroutine runs next. This keeps libultra's run-to-block, priority-ordered behaviour, which
the game relies on: it has one `osSetIntMask` critical section and no locks. Native preemptive threads would
introduce data races that the original never had.

Hardware events become HAL posts into the queues registered with `osSetEventMesg` and `osViSetEvent`:

- the platform's frame timer posts the retrace message at 60 Hz;
- the renderer posts "RSP done" and "RDP done" when it has drawn a list;
- the audio backend posts "audio task done".

`Main_Thread` remains the scheduler. `osSpTaskLoad`/`osSpTaskStartGo` with a graphics task call
`hal_gfx_run( list )`, and with an audio task call `hal_audio_run( list )`. Both run synchronously, so the
yield path (`osSpTaskYield`) becomes a no-op that reports "not yielded". A later cleanup can replace the coroutine
layer with an explicit host frame loop, one game frame per two retraces, once the port is stable.

### Overlays

**Recommended: link everything.** All seven overlays and main go into one executable, since the symbol names
are already unique. `Game_Thread`'s load step (finding the file in `BW64FAT`, DMA into the slot, invalidating the
caches and calling the slot address) becomes a table `{ init, frame }` indexed by `SCENE_*`.

One behaviour must be kept: on the N64, loading an overlay **restores its `.data` and clears its `.bss`** each
time its scene starts. The port places each overlay's data and bss in named sections (or one object library per
overlay with linker-provided bounds). At startup it snapshots each overlay's initial `.data` image, and on each
scene change it copies the snapshot back and zeroes the overlay's bss, as `Game_Thread`'s load
does today.

### Memory

- **The heap:** `HEAP_START`..`HEAP_END` becomes a host allocation of the same size (about 2.3 MB at retail
  addresses; doubling it is cheap and absorbs the growth of 64-bit structs). `Mem_*` (`src/51F0.c`) keeps its
  first-fit allocator over that arena.
- **The framebuffers and the depth buffer** become renderer targets. `sched_framebuffers[i]` hold opaque handles,
  and `Dbg_ScreenShot` reads back through `hal_gfx_read_pixels`.
- **The boot `memset`** from `boot_staticFile` to `RAM_END` becomes zeroing of the static regions that it covered
  (the file table, `KNSTATICSYS`, the RDP FIFO, the overlay sections). The host already zeroes bss.
- **`osVirtualToPhysical` (21 sites), the cache calls (22) and `K0_TO_PHYS`** become identity or no-ops. The
  renderer and the audio code take host pointers directly.

### File I/O and the ROM

**Recommended:** `hal_rom` opens the user's ROM, checks its SHA-1 and maps it read-only. PI DMA (`osEPiStartDma`,
5 sites) becomes a `memcpy` from that image, given the ROM offset the game already computes. `FAT_ReadFile`,
`Str_ReadFile`, `LZ_Decompress` and the audio sample DMA (`src/1A7A0.c`) then work unchanged. As an option, the
port reads the extracted tree (`assets_ext/`, `manifest.json`) from `tools/assets.py`, so that edited assets load
without a repacked ROM. Two sources change role:

- The overlay files and `KNSTATICSYS` are no longer read: the overlays are linked code, and `KNSTATICSYS` (8 small
  display lists and a viewport) becomes a C data unit built with `gsSP*`/`gsDP*` initialisers.
- The development host link (`src/4890.c`, PI registers at `0x11000000`, `__osEPiRaw*`) becomes file access in a
  `host/` directory, behind `cache_hostMode`, or is left out.

### Data loading: endianness and pointer width

This is the largest piece of work, and the reason for a 64-bit design.

- **Recommended: explicit load-time conversion at the existing relocation points.** All in-place relocation
  already happens in a few places: `Cache_GetFile` (one case per `FILE_TYPE*`), `Snd_RelocateWaveBank`,
  `Snd_RelocateTable` (sequences, SE tables) and `Sound_Init`'s bank tables. In the port, each of those reads
  the big-endian disk layout (`u32` offsets) and builds a runtime struct with native pointers. The disk layout is
  declared as a separate `*_disk_t` with fixed-width fields and offset comments, and the runtime struct as the
  type the game already uses. The game code past those functions keeps using the runtime types.
- **Byte swapping** happens in the same converters: header words, `Vtx` fields, `f32` animation and camera data,
  palettes, sequence word tables, ADPCM books. Streams that are read byte by byte (sequence byte code, LZSS, CI4/CI8
  pixels) stay as they are.
- **`Gfx` on 64-bit:** the port's `Gfx` is `{ u32 w0; uintptr_t w1; }`, so that addresses fit. `Obj_ConvertModel`
  can no longer convert 8-byte model commands in place, so its port version writes into a separately allocated
  list. That is a measured, single-function change.
- **The 32 pointer-to-integer casts** become `uintptr_t` (or go away once the converters produce pointers).
- **Alternative, for an early bring-up only:** a 32-bit build (`-m32` on Linux, Win32 on Windows) avoids the
  pointer-width work and allows the in-place relocation. macOS has no 32-bit target, so this cannot be the end
  state.

### Rendering

The game emits F3DEX2 display lists through ultralib's `gbi.h` macros: 290 sites and 49 distinct macros, plus lists
held as data (models, `KNSTATICSYS`) and combiner state read from materials. There are two ways to replace the RSP
and RDP:

| | A. native renderer for the game's GBI commands | B. replace the drawing calls |
|---|---|---|
| what changes | a renderer that walks the `Gfx` lists and turns each command into native draw state and draw calls | `Scene_Draw`, `Draw_*`, `Txt_*`, `Menu_*`, `Score_*` and model loading call `R_*` HAL functions directly; meshes become native vertex buffers at load |
| game code touched | none outside `Obj_ConvertModel` and the `Gfx` type | every one of the 290 sites, the model converter, and the material path |
| covers data lists | yes | only after models and `KNSTATICSYS` are converted to native meshes and state |
| first frame | early: all scenes draw at once | late: one subsystem at a time |
| risk | creeping into RDP emulation (TMEM images, per-pixel blender) | divergence in the many hand-tuned 2D draw paths |
| end-state readability | the game's lists remain the interface | idiomatic native drawing |

**Recommended: A first, with a strict boundary, then B where it pays.** The renderer supports exactly the
command set the game sends (the 49 macros plus the 4 model commands; anything else is a hard error during
development). It translates them as follows:

- vertices and matrices go to a native vertex buffer and transform, with the 32-vertex cache that `gSPVertex`
  indexes;
- `gDPSetCombineLERP` and `gDPSetRenderMode` go to generated shaders and native blend and depth state;
- `gDPSetTextureImage` plus the `LoadBlock`/`LoadTextureBlock`/`SetTile` group go to a texture cache keyed by
  source pointer, format and size, decoded on the CPU from CI4, CI8, I4, I8 and RGBA16 with the TLUT;
- rectangles (`gSPTextureRectangle`, `gSPScisTextureRectangle`, `gDPFillRectangle`) go to screen-space quads;
- `gSPLoadUcodeL` goes to a mode flag (triangles, or lines for `L3DEX2`), and the line commands go to native lines.

The renderer models no TMEM byte image, no DMEM, no RDP FIFO, no cycle timing and no coverage or dither
behaviour beyond what the measured render modes visibly need. Its command semantics are written from the game's
use and from the public command formats that `gbi.h` encodes, not from any microcode. After that, the 2D paths
(`Draw_*`, `Txt_*`), which build their lists in code and split images into texture-memory strips
(`Draw_CiImage`), are good candidates for B: each becomes one textured quad.

Resolution: the game draws to 320x240. The renderer can scale the native target. 2D coordinates are in
screen pixels (10.2 fixed point in rectangles), so widescreen output needs per-scene decisions and is out of
scope for a first port.

### Audio

**Recommended: run the game's sequencer and synthesizer natively, and replace only the RSP stage.** The
sequencer (`Snd_*`, `src/166B0.c`) and the synthesizer (`Synth_*`, `n_alSyn*`, `al*`) are game C and compile
as they are. They produce a list of the 15 commands above for each audio frame. The port replaces the RSP step in one of two ways:

- **Execute the list** in `hal_audio_run`: a native implementation of the 15 commands over a working buffer
  addressed by the commands' offsets (ADPCM decode with the bank's book and loop state, resampling, envelope mix
  with the volume ramps, the pole filter, buffer moves, interleave). The result is a stereo 16-bit frame that
  goes to the audio device. This needs no change in the synthesizer.
- **Or redefine the command macros** of `include/n_libaudio.h` under `TARGET_NATIVE` so that each one calls its
  native DSP function at once instead of appending to a list. That removes the list altogether and is the
  cleaner end state.

Either way, the semantics come from how `src/19F50.c`-`src/1E140.c` use each command (buffer sizes, state
pointers, the `A_INIT`/`A_CONTINUE`/`A_LOOP` flags) and from the public command layout in `include/n_libaudio.h`.
`osAiSetFrequency` becomes the device rate (22050 Hz, or host resampling), `osAiSetNextBuffer` queues a frame,
and `osAiGetLength` returns the queued sample count, so `Snd_NextFrameSize` keeps the queue steady with the
host device instead of the AI. Sample DMA from `bgmwbk`/`sewbk` reads the mapped ROM.

### Input and rumble

**Recommended:** `hal_input` fills the game's `OSContPad` (buttons, `stick_x`, `stick_y`) for the two ports that
the game reads (`NUM_PLAYERS`) from SDL gamepads or the keyboard. It maps N64 buttons to configurable bindings and
scales the stick to the N64's range of about ±80, since `Cont_ReadPads` applies its own dead zone. `osContInit` and
the query calls report the connected pads. The Rumble Pak (`osMotorInit`, start and stop) maps to SDL rumble; the
pulse pattern that `cont_rumble` produces is kept as it is.

### Saves

**Measured:** the game saves one 0x100-byte `saveData_t`, with its own checksum, in a Controller Pak note of game code
`NBWE`. It has no EEPROM or SRAM saves. All Pak access goes through `Cont_Thread`'s request queue
(`contRequest_t`: find, allocate, read, write, delete, count notes and free space, open, repair, probe rumble).

**Recommended:** implement the request handler's Pak operations over a file, one note per port, for example
`saves/NBWE-port1.bin`, holding the 0x100 bytes in their big-endian on-cartridge layout so that `Save_Checksum`
and the loaders work as they are. `osPfsNumFiles`/`osPfsFreeBlocks` report one note and plenty of free space.
`OP_PakScreen`, which lists the notes of a pak, shows only the game's own note. A full Controller Pak image
format is not needed.

### Build

**Recommended:** a CMake project (`port/CMakeLists.txt`) that compiles `src/*.c`, `src/<overlay>/*.c` and the
HAL with the host compiler, with `-DTARGET_NATIVE -DNON_MATCHING`. The settings:

- `-fno-strict-aliasing` and `-fwrapv`, since the original was compiled by gcc 2.7.2 without type-based alias
  analysis, and signed overflow wraps on MIPS;
- `-ffp-contract=off`, so that no FMA changes `f32` results;
- `-funsigned-char` must not be given: plain `char` is signed, as on the N64;
- sanitizers (ASan, UBSan) in debug builds;
- warnings for pointer truncation (`-Wpointer-to-int-cast`, `/W4` on MSVC).

Linked from the project's sources: libkmc's C (`src/libkmc/*.c`, the project's reconstruction), so that `atan`
(CORDIC), `sqrt`, `rand` and `asin`/`acos` give the original's results; and the `gu*` and `sinf`/`cosf`
functions the game calls, either compiled from ultralib's sources (a decision for the maintainers, recorded with
the port) or written natively and checked against values captured from the matching build. `guRandom` (22
sites) decides gameplay randomness, so its sequence must be reproduced exactly. The matching `Makefile` and the
gate are untouched. The port is a second consumer of the same `src/`, and any `#ifdef TARGET_NATIVE` in game code must
leave the matching build's bytes unchanged (`make gate`).

## Staged plan (recommended)

1. **Compile and link.** Build all of `src/` natively against stub HAL functions that return success. Fix the
   pointer-width and type errors that the compiler and `-Wpointer-to-int-cast` report. Run the coroutine
   scheduler with a 60 Hz retrace and no output. Exit criterion: `Game_Thread` reaches `OP_Init` and loops through
   `OP_Frame` under ASan without faults.
2. **ROM and data.** `hal_rom` with the SHA-1 check, then the disk-to-runtime converters for `IDX     SYS`, images
   and the `FILE_TYPE1`-`FILE_TYPE3` files. Exit criterion: every str member that the opening and game select
   request loads and converts.
3. **First frame (the milestone).** `hal_gfx` with path A for the subset that the opening's logo and title
   screens use: 2D rectangles, CI4/CI8 images, fill and copy modes, the `KNSTATICSYS` lists. Exit criterion: the
   logos and the title screen draw correctly in a window at 30 frames a second. Then extend to the 3D path
   (`Scene_Draw`, models, cameras, fog, lines) until the demo game runs.
4. **Input.** `hal_input` and rumble. Exit criterion: a full game can be played from game select to results.
5. **Audio.** `hal_audio_run` executing the command list, with the AI calls on an SDL audio stream. Exit
   criterion: BGM and effects play at the correct pitch, and the queue stays steady when the window's refresh rate changes.
6. **Saves.** Controller Pak requests over a file. Exit criterion: settings and records survive a restart, and a
   corrupted file goes through the game's own repair prompt.
7. **Cleanup toward B.** Native 2D drawing paths, the audio macros as direct calls, and the coroutine layer
   replaced with a host frame loop, as each one proves equivalent.

## Risks

| risk | where (measured) | mitigation |
|---|---|---|
| pointer width | 93 pointer fields in file and sound structs, in-place relocation (13 `FILE_RELOC`, 7 `Snd_RelocateTable` calls), 32 pointer-to-int casts, 32-bit `Gfx.w1` | disk/runtime struct split in the converters; widened `Gfx`; a 32-bit build only for bring-up |
| endianness | every asset format; `Vtx`, `f32` data, ADPCM books, save block | swap in the converters only; keep save files in the cartridge layout |
| undefined behaviour the original relied on | `Obj_ConvertModel` copies 0x18 bytes from each 8-byte command, which reads past the end of the last one; float-to-unsigned casts of parameter floats (`(u32)params->args[5]`); writes through structs overlaid on file data (aliasing); the 11 `R36` quirks | pad file buffers; `-fno-strict-aliasing -fwrapv`; check float-to-int conversions under UBSan and match MIPS truncation where values can be negative; keep the R36 behaviour |
| unproven drafts | 19 `NON_MATCHING` functions, including `Game_Thread` and `Synth_DecodeSamples` | finish matching them before relying on them; suspect them first |
| timing | logic counted in 30 Hz frames, audio woken every other retrace, frame sizes tied to the AI queue | fixed 30 Hz simulation step, 60 Hz retrace tick, audio paced by the device queue; never tie the step to the monitor's refresh rate |
| floating point | single precision (`-mfp32`), gcc 2.7.2's promotions to `double` in literals, `guMtxF2L` fixed point | compile with the same literal types as `src/`; no FMA; the gu math from one source |
| overlay state | each scene start restores `.data` and clears `.bss` | per-overlay section snapshots |
| stealth emulation | the GBI renderer and the audio command executor | the closed measured command sets above; no DMEM/TMEM/register model; migrate to B over time |
| legal | ROM data, SDK material | the user's ROM only, checked by hash; no SDK sources, headers, samples or microcode used or imitated |
