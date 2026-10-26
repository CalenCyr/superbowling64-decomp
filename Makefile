BASENAME := superbowling
BUILD    := build
ROM      := $(BUILD)/$(BASENAME).z64
ELF      := $(BUILD)/$(BASENAME).elf
LD_SCRIPT := $(BASENAME).ld
SHA1     := 56937a9e9536af55ebed06480413265848c4a04a

CROSS    := mips64-linux-gnu-
AS       := $(CROSS)as
LD       := $(CROSS)ld -m elf32btsmip
OBJCOPY  := $(CROSS)objcopy

ASFLAGS  := -EB -march=vr4300 -mabi=32 -G0 -I include

# KMC GCC 2.7.2; its driver runs KMC as 2.6, which the matching code needs (e.g. `move` -> addu).
GCC_DIR  := tools/gcc_2.7.2
CC       := COMPILER_PATH=$(GCC_DIR) $(GCC_DIR)/gcc -B$(GCC_DIR)/
CFLAGS   := -c -G0 -mips3 -mgp32 -mfp32 -O2 -nostdinc -I include -I src -I lib/ultralib/include
# KMC's libkmc was built with -O, not the game's -O2: its code is unscheduled (rand stores its state before it
# computes the result; -O2's scheduler moves the store last).
$(BUILD)/src/libkmc/%.o: CFLAGS := $(subst -O2,-O,$(CFLAGS))

# libultra: decompals ultralib (git submodule), built as 2.0K libgultra_rom with the same KMC compiler. splat `lib`
# subsegments place its members; the link pulls them from the archive by symbol, as the original link did.
# MDEBUG_FILES= assembles monutil.s (IDO-only, unused here) with KMC as instead of IDO.
ULTRALIB  := lib/ultralib
LIBULTRA  := $(BUILD)/ultralib/K/libgultra_rom/libgultra_rom.a
ULTRALIB_FLAGS := TARGET=libgultra_rom VERSION=K COMPARE=0 CROSS=$(CROSS) COMPILER_DIR=$(CURDIR)/$(GCC_DIR) \
	MDEBUG_FILES= BUILD_ROOT=../../$(BUILD)/ultralib

S_FILES   := $(shell find asm -name '*.s' -not -path 'asm/nonmatchings/*' 2>/dev/null)
C_FILES   := $(shell find src -name '*.c' 2>/dev/null)
BIN_FILES := $(shell find assets -name '*.bin' 2>/dev/null)
O_FILES   := $(S_FILES:%.s=$(BUILD)/%.o) $(C_FILES:%.c=$(BUILD)/%.o) $(BIN_FILES:%.bin=$(BUILD)/%.o)

.PHONY: all split gate clean expected report diff rules rules-test FORCE

OBJDIFF  := tools/objdiff/objdiff-cli

all: $(ROM)

split:
	python3 -m splat split $(BASENAME).yaml

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD)/%.o: %.bin
	@mkdir -p $(dir $@)
	$(LD) -r -b binary -o $@ $<

$(LIBULTRA): FORCE
	$(MAKE) -s --no-print-directory -C $(ULTRALIB) $(ULTRALIB_FLAGS) -j$(shell nproc)

$(ELF): $(O_FILES) $(LIBULTRA) $(LD_SCRIPT)
	$(LD) -T $(LD_SCRIPT) -T undefined_syms.txt -T undefined_syms_auto.txt -T undefined_funcs_auto.txt \
		--no-check-sections -Map $(BUILD)/$(BASENAME).map -o $@ $(LIBULTRA)

$(ROM): $(ELF)
	$(OBJCOPY) -O binary $< $@

# The byte gate: clean rebuild into a fresh build/, judged by exit codes and the hash.
gate:
	rm -rf $(BUILD)
	$(MAKE) --no-print-directory all
	@echo "$(SHA1)  $(ROM)" | sha1sum -c --quiet - && echo "GATE OK: $(ROM) matches retail SHA-1 $(SHA1)"

clean:
	rm -rf $(BUILD)

# Snapshot a green build as objdiff's target objects.
expected: gate
	rm -rf expected && mkdir -p expected && cp -r $(BUILD) expected/$(BUILD)

objdiff.json: $(C_FILES) $(LIBULTRA) superbowling.yaml tools/objdiff_config.py
	python3 tools/objdiff_config.py

report: all objdiff.json
	$(OBJDIFF) report generate -o report.json
	-python3 tools/rules.py --no-advisory --report report.json --json $(BUILD)/rules.json > /dev/null
	python3 tools/progress.py report.json --rules $(BUILD)/rules.json

# Rule checker: make rules [UNIT=src/<unit>] (all src/**/*.c by default); exits non-zero on [match] findings.
rules:
	python3 tools/rules.py $(UNIT)

rules-test:
	python3 tools/rules_tests/run.py

# make diff UNIT=src/166B0 FUNC=func_8003C0B0  (text diff; `$(OBJDIFF) diff -u <unit> <func>` for the TUI)
diff: all
	python3 tools/fdiff.py $(BUILD)/$(UNIT).o $(FUNC)

# Per-unit optimisation: these units inline their own earlier functions at their call sites (-O3 in gcc 2.7.2).
$(BUILD)/src/4890.o: CFLAGS += -O3
