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

S_FILES   := $(shell find asm -name '*.s' -not -path 'asm/nonmatchings/*' 2>/dev/null)
BIN_FILES := $(shell find assets -name '*.bin' 2>/dev/null)
O_FILES   := $(S_FILES:%.s=$(BUILD)/%.o) $(BIN_FILES:%.bin=$(BUILD)/%.o)

.PHONY: all split gate clean

all: $(ROM)

split:
	python3 -m splat split $(BASENAME).yaml

$(BUILD)/%.o: %.s
	@mkdir -p $(dir $@)
	$(AS) $(ASFLAGS) -o $@ $<

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
