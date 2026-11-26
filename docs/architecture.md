# How the game is put together

This describes Super Bowling 64 as the reconstructed source shows it. Function and global names are the
project's: a few come from evidence (ROM strings, libultra and n_audio), most are invented from behaviour, and
`config/names.txt` records which is which. Names of the form `func_8XXXXXXX` / `D_8XXXXXXX` are still unnamed.
Open questions are marked **Open**.

## ROM and memory

The ROM holds one code segment that stays resident (`main`), a file table, a static data file, seven code
overlays and the asset files. `superbowling.yaml` is the authoritative map; in short:

| ROM | contents | runs at |
|---|---|---|
| `0x1060` | `main`: the game's resident code, libultra 2.0K, KMC's libkmc, the `rspboot` and F3DEX2 2.08 microcodes | `0x80025C60` |
| `0x3C2A0` | `BW64FAT`, the file table | `0x8010A080` |
| `0x3C470` | `KNSTATICSYS`, static data, including the display lists each frame starts with | `0x8010A250` |
| `0x3C570`-`0x6DE70` | seven code overlays (`GACODE  SYS` ... `OPCODE  SYS`) | `0x80112350`, one at a time |
| `0x6DE70`-`0x656CD0` | asset files: `idx`, `str`, `bgmptr`, `bgmwbk`, `septr`, `sewbk`, `bfx` | `idx` and the sound tables at boot, the rest on demand |

RDRAM is 4 MB (`RAM_END` 0x80400000). From the source:

- `0x8010A080`: the file table (`fat_table`), then at `0x8010A250` `KNSTATICSYS` (`boot_staticFile`); `main`
  clears memory from `boot_staticFile` to the end of RAM at boot, and `Main_Thread` then loads both files.
- `0x8010A350`-`0x80112350`: the RDP's output FIFO (`SCHED_FIFO_START`, `SCHED_FIFO_END`).
- `0x80112350`: the overlay slot, with each overlay's bss after its code.
- `0x80150000`-`0x8038F7FF`: the heap (`Mem_Reset` in `main`; allocator in `src/51F0.c`, first fit, 8-byte
  granularity, blocks cleared on allocation).
- `0x8038F800`, `0x803B5000`, `0x803DA800`: three 320x240 RGBA5551 framebuffers (`sched_framebuffers`).

## Boot and threads

`main` (`src/1060.c`) initialises libultra, clears memory, sets up the heap and the players' records, and starts
the boot thread. The threads, by libultra thread id:

| id | thread | priority | job |
|---|---|---|---|
| 1 | `Boot_Thread` | 10 | starts the VI and PI managers and the main thread, then idles |
| 3 | `Main_Thread` | 20 | loads the file table, `IDX     SYS` and `KNSTATICSYS`, starts the other threads, then schedules the RSP and RDP for ever |
| 2 | `Cont_Thread` (`src/2DF0.c`) | 0x11 | reads the pads, drives the Rumble Paks, serves Controller Pak requests |
| 5 | `Game_Thread` | 8 | builds one frame's display list each time the main thread asks; loads and runs the scenes |
| 3 | `Audio_Thread` (`src/1ADA0.c`) | 18 | synthesizes each audio frame (created by the sound driver with `AUDIO_THREAD_ID` 3) |

### The scheduler

There is no separate scheduler thread: `Main_Thread` is the scheduler. Its queue `sched_queue` receives the
retrace, RSP-done, RDP-done and pre-NMI events and two requests, each message pointing to its code
(`SCHED_MSG_*` in `include/boot.h`):

- `SCHED_MSG_GFX`: the game thread finished a display list. The game thread builds into one of two
  `sched_gfxFrames` (each 0x3000 `Gfx`, with its size and target framebuffer); the scheduler starts the other on
  the RSP as a graphics task once its framebuffer is neither shown nor queued for display.
- `SCHED_MSG_AUDIO`: the audio thread wants its task run. If the RSP is busy with graphics the scheduler asks it
  to yield (`osSpTaskYield`) and runs the audio task (`Main_StartAudioTask`) when it has.
- On retrace it shows the next drawn framebuffer, at most every second retrace (so the game runs at up to 30
  frames a second on NTSC), counts frames (`sched_frameCount`) and swaps per second (`sched_frameRate`), and every
  other retrace wakes the audio thread through `sound_frameQueue`.
- Whenever no list is being built and the other list has finished, it picks the next framebuffer and list and
  tells the game thread (`game_drawQueue`) to build it.

It measures the CPU, RSP and RDP time of each frame (`sched_cpuTime`, `sched_rspTime`, `sched_rdpTime`), which the
debug scenes print. After the reset button (`SCHED_MSG_PRENMI`) it starts no new tasks.

### The game thread's frame

Each frame `Game_Thread` moves the on-screen cursor by the stick, starts the display list from the
`KNSTATICSYS` lists, and when `game_scene` has changed: draws two blank frames, resets the heap, the file cache
and the object list, clears the new overlay's bss, reads its file into the overlay slot, invalidates the caches
and calls the overlay's `XX_Init`. Then it calls the scene's `XX_Frame`, draws a demo's titles, applies the fade
of `game_leaveState` (a grey fill whose alpha also lowers the BGM's master volume) and sends the list to the
scheduler.

## Overlays and scenes

All seven code overlays are linked at `0x80112350`; only one is in memory at a time. `main` finds each overlay
file by its 8.3 name in the `BW64FAT` table (`FAT_GetFileSize`, `FAT_ReadFile` in `src/3AD0.c`: an 11-character
name, ROM start and end per entry, DMA in 0x2000-byte chunks). The overlay symbols carry a `_<segment>` suffix
(`GA_TurnFirstBall`), since the overlays share addresses.

`game_scene` picks the overlay (`SCENE_*` in `include/overlay.h`); it starts at `SCENE_OPCODE`.

| scene | file | source | what it is |
|---|---|---|---|
| 3 | `OPCODE  SYS` | `src/opcode/` | the opening: two logo screens, the title with its demo games, the settings check at boot (`OP_CheckSettings`) and the Controller Pak screen (`OP_PakScreen`); then game select |
| 4 | `GSCODE  SYS` | `src/gscode/` | game select: a stack of menu screens (`GS_PushScreen`/`GS_PopScreen`) for the game mode, players, characters, team select, options, records, credits and the Controller Pak check; then the game |
| 0 | `GACODE  SYS` | `src/gacode/` | the game itself: lanes, players, throws, pin physics, scoring per game mode, panels and the pause menu; then results, or back to game select |
| 6 | `RSCODE  SYS` | `src/rscode/` | results: score sheets, pin diagrams, records, duel results; adds the game to the records and saves them |
| 1 | `CACODE  SYS` | `src/cacode/` | a motion editor for a scene: steps and copies frames of the scene's animation, shows clips of BMP frames, saves its tables as `motion.dat` |
| 2 | `PVCODE  SYS` | `src/pvcode/` | a scene viewer: runs a scene's objects, with free cameras (C buttons), the floor grid editor, the save screen and sound tests |
| 5 | `MGCODE  SYS` | `src/mgcode/` | builds and draws a random 21 x 21 maze |

The title screen starts a demo game after 450 frames: `OP_Frame` sets `cont_demoMode` and switches to `gacode`
(during a demo `Cont_ReadPads` fakes R presses, and Start cuts the demo short), and `Game_Thread` draws the demo's
titles and returns to the opening when its frames run out.

**Open:** nothing in the reconstructed source sets `game_scene` to 1, 2 or 5, so how `cacode`, `pvcode` and
`mgcode` were reached (a development build, a debugger, or a path not yet found) is unknown. Several features
point to development use: `cache_hostMode`, the host link, `Dbg_ScreenShot` and the grid editor.

`src/3990.c` (`Ovl_KeepRoutines`) is never called; it refers to library routines that only the overlays use, so
that the link of `main` keeps them.

## Graphics

- **Microcodes:** the graphics task boots with `rspboot` and starts in `gspF3DEX2_Rej_fifo`; display lists switch
  between `gspF3DEX2_Rej_fifo`, `gspF3DEX2_fifo` and `gspL3DEX2_fifo` (lines, used by the debug grid editor, the
  cacode editor and `Draw_Line3D`) with `gSPLoadUcodeL`. All are F3DEX2 2.08 FIFO versions; output goes through
  the RDP FIFO above.
- **2D drawing** (`src/7250.c`, `Draw_*`): images of the file cache in copy or one-cycle mode, drawn in strips that
  fit texture memory; rectangles, outlines, a rounded box, screen clears. `draw_rdpMode` remembers the 2D mode
  last set up, to skip redundant state.
- **Text** (`src/3D50.c`, `Txt_*`): an 8x8 font of characters 0x20-0x5F (debug text) and an 8x16 glyph sheet loaded
  from a file, for strings of glyph indices.
- **Menus and the scoreboard** (`src/13490.c`, `Menu_*`, `Score_*`): image menus that slide in from an edge, name
  plates, the bowling scoreboard and score cards, record tables.

### Objects, scenes and cameras

A scene is a list of objects created from parameter records:

- **Classes.** Each overlay has a table of classes (`obj_classes`), each a `create` and an `update` function.
  A record (`objParams_t`, sixteen floats) names its class by index and gives a priority; `Obj_CreateAll`
  (`src/53B0.c`) creates an object for each record until one of type -1. A record of type 2 is drawn as an image
  instead.
- **The object list** (`obj_list`) is kept in priority order; `Obj_Frame` calls every object's `update` each
  frame, then draws the scene.
- **Scene objects** (`sceneObject_t`) hold a model (a `FILE_TYPE1` file: parts with joints, meshes and
  materials) and an animation (`FILE_TYPE2`: per frame, the joint components each joint's mask selects).
  `Obj_PoseJoints` builds the joint matrices, `Obj_ConvertModel` turns the model's compact commands into a
  display list in place, and `Obj_AddToDrawList` queues each part for each camera that sees it, with an optional
  mirror image on a reflective floor.
- **Cameras** (`Cam_*` in `src/7250.c`) are objects too. Each follows a camera path file (`FILE_TYPE3`: per frame
  eye, target and up point, field of view, fog, light). Matrices are double-buffered by `sched_bufferIndex`.
- **The draw list** (`Scene_*` in `src/C650.c`) is sorted into 0x400 buckets by key (layer and material or
  microcode). `Scene_Draw` walks it, setting up each entry's microcode, camera, render modes, combiner and
  textures only when they change. `Scene_*Puff*` draws camera-facing squares that grow and fade.

## Input, rumble and the Controller Pak

The controller thread (`Cont_Thread`, `src/2DF0.c`) runs each retrace:

- It reads both ports into per-port state rebuilt by `Cont_ReadPads`: buttons held and pressed, the stick outside
  its dead zone, the stick as D-pad bits, auto-repeating D-pad presses, and the two ports merged
  (`cont_allHeld`, ...). Only two ports are read (`NUM_PLAYERS`).
- It drives the Rumble Pak of one port per frame, alternating: a strength (`cont_rumble`) for a number of frames,
  run as pulses of the motor.
- It serves one request at a time from `cont_requestQueue` (`contRequest_t` in `include/input.h`): find,
  allocate, read, write or delete the game's Controller Pak note, count notes and free space, open or repair a
  pak, probe for a Rumble Pak. Other threads post a request and wait on `cont_replyQueue`.

`OP_PakScreen` lists and deletes notes of a pak (the Controller Pak screen at boot).

## Save data

The game saves one 0x100-byte block, `saveData_t` in `include/boot.h`: a checksum of bytes 0x04-0xFF
(`Save_Checksum`), character unlock bits, four player records (games, total, high and low score, strikes,
spares, splits) and the options (difficulty, music, music volume, rumble). It lives in a Controller Pak note of
game code `NBWE`. At boot `OP_CheckSettings` looks for the note and offers to load, create or repair it;
`Save_SaveSettings` (`src/12260.c`) writes it, and `Save_ShowPrompt` runs the game's yes/no prompts. Records of
the current session (`game_sessionRecords`) are not saved.

**Open:** `Sram_GetHandle` (`src/47E0.c`) sets up an SRAM PI handle, but no caller has been found.

## Files: the file table, the str segment and the cache

- **`BW64FAT`** names the files the code opens by name, among them the overlays, `IDX     SYS`, `KNSTATICSYS`
  and the sound files `BGMPTR  SYS`, `SEPTR   SYS` and `BFX     SYS`.
- **The str segment** holds the other assets (images, models, animations, camera paths, BGM sequences), by
  file index. `IDX     SYS` is its table (`str_files`, three words per file: offset, stored size, decompressed
  size). `Str_ReadFile` (`src/4DC0.c`) reads a file and, when the sizes differ, decompresses it with
  `LZ_Decompress`: LZSS with a 4 KB window, matches of 3 to 18 bytes.
- **The file cache** (`src/BEF0.c`) holds up to 0x1000 files by index, reference-counted, on the heap.
  `Cache_GetFile( index, type )` loads a file on first use and turns its stored offsets into pointers by type:
  `FILE_RAW`, a model (`FILE_TYPE1`, which also loads the mesh and material files its parts name), an animation
  (`FILE_TYPE2`) or a camera path (`FILE_TYPE3`). `Cache_Release` drops a reference. When `cache_hostMode` and
  `cache_hostFiles` are both set, files come from the development host as `data.NNNN` instead.

### The development host

`src/4890.c` is a byte link to a development host through PI registers at `0x11000000`: a handshake, then
requests, each a header with a command, a size and a file name (`HostPacket`): 1 names a file, 2 returns a file's
size and then its bytes, 3 writes a file. The game uses it to read host files
(`Host_ReadFileToObjBuf`, the viewers' `sce.dat`), to save the floor grid (`ground.bin`, `src/111A0.c`) and the
motion tables (`motion.dat`), and for screenshots: `Dbg_ScreenShot` (`src/13230.c`) writes the screen as
`BWnnn.BMP` on Z + L while `cache_hostMode` is set.

## Sound

Three layers, from the game down:

1. **The game's sound calls** (`src/12C70.c`, `Sound_*`): `Sound_Init` loads the wave bank tables (`BGMPTR  SYS`
   for music, `SEPTR   SYS` for effects, whose samples stay in the `bgmwbk` and `sewbk` ROM segments) and the
   effect table (`BFX     SYS`), and starts the driver at 22050 Hz with 32 voices. `Sound_PlayBgm` reads a BGM
   sequence from the str segment into one of two buffers and starts it a few frames later; `Sound_PlaySe` plays a
   sound effect. The game's audio task routines send the audio task through the main thread's scheduler.
2. **The sequencer driver** (`src/166B0.c`, `Snd_*`), the game's own: a byte-code sequencer for BGM and sound
   effects. Calls from other threads post messages into a ring buffer (`Snd_PostMessage`), run on the next tick.
   `Snd_PlayerHandler` runs once a tick as an n_audio client player; each channel reads one command stream:
   notes (with optional velocity and length) and commands from `CMD_FIRST` on, dispatched through
   `snd_commands`. Channels have envelopes, auto-pan, vibrato and trill, volume and pitch-bend streams, drum
   maps, and per-sound volume, pan, pitch, tempo and effect mix. The data formats are documented in
   `include/structs.h` (`seqHeader_t`, `envelope_t`, `drumEntry_t`, ...).
3. **The synthesizer** (`src/19F50.c` to `src/1E140.c`, `Synth_*` and the `n_alSyn*` calls): the n_audio
   synthesizer, reconstructed clean-room from the ROM. Physical voices pull samples through ADPCM or raw
   decoders, a resampler and an envelope mixer; one effect bus runs a delay-line effect whose storage is sized for
   the largest of its presets, so the preset can change while sound plays. Around it, the game's audio code:
   `src/1A7A0.c` caches ROM samples in DMA buffers, `src/1B060.c` is the audio heap, `src/1B1A0.c` sizes each
   audio frame to keep the AI queue steady, and `Audio_Thread` (`src/1ADA0.c`) builds each frame's command list
   and runs it on the RSP with the `aspMain` microcode.

## Libraries

- **libultra 2.0K** (`libgultra_rom`) is linked from decompals' ultralib source (`lib/ultralib`), each member at its
  retail address.
- **libkmc** (`src/libkmc/`) is KMC's C runtime, linked after libultra: string and memory routines, `atoi`,
  `rand`, a CORDIC `atan`, `asin`/`acos`, `sqrt`, ctype, `errno` and the SVID `matherr` hook. Its 64-bit division
  helpers (`llmath.c`) were hand-written assembly in the original and stay assembly.
