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
CFLAGS   := -c -G0 -mips3 -mgp32 -mfp32 -O2 -nostdinc -I include -I src

S_FILES   := $(shell find asm -name '*.s' -not -path 'asm/nonmatchings/*' 2>/dev/null)
C_FILES   := $(shell find src -name '*.c' 2>/dev/null)
BIN_FILES := $(shell find assets -name '*.bin' 2>/dev/null)
O_FILES   := $(S_FILES:%.s=$(BUILD)/%.o) $(C_FILES:%.c=$(BUILD)/%.o) $(BIN_FILES:%.bin=$(BUILD)/%.o)

.PHONY: all split gate clean expected report diff

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

$(ELF): $(O_FILES) $(LD_SCRIPT)
	$(LD) -T $(LD_SCRIPT) -T undefined_syms.txt -T undefined_syms_auto.txt -T undefined_funcs_auto.txt \
		--no-check-sections -Map $(BUILD)/$(BASENAME).map -o $@

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

objdiff.json: $(C_FILES) tools/objdiff_config.py
	python3 tools/objdiff_config.py

report: all objdiff.json
	$(OBJDIFF) report generate -o report.json
	python3 tools/progress.py report.json

# make diff UNIT=src/166B0 FUNC=func_8003C0B0  (text diff; `$(OBJDIFF) diff -u <unit> <func>` for the TUI)
diff: all
	python3 tools/fdiff.py $(BUILD)/$(UNIT).o $(FUNC)
