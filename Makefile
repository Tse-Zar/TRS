# =============================================================
# = TRS - Tsezar Reliable System | build system               =
# =  build/BOOTX64.EFI  - UEFI loader (GNU-EFI, PE32+)        =
# =  build/kernel.bin   - kernel flat binary                  =
# =  build/trs.img      - GPT disk + FAT32 ESP                =
# =============================================================

# ---------------- tools ----------------
CC      ?= cc
LD      := ld
OBJCOPY := objcopy
NASM    := nasm
QEMU    ?= qemu-system-x86_64
PARTED  ?= parted
MKFAT   ?= mkfs.vfat
MMD     ?= mmd
MCOPY   ?= mcopy
DD      := dd
TRUNC   := truncate
CP      := cp

# ---------------- gnu-efi (Arch Linux) ----------------
GNUEFI  ?= /usr/lib
EFICRT0 := /usr/lib/crt0-efi-x86_64.o
EFILDS  := /usr/lib/elf_x86_64_efi.lds
EFIINC  := /usr/include/efi

# ---------------- ovmf (для make run) ----------------
OVMF_CODE := $(firstword $(wildcard \
                /usr/share/ovmf/x64/OVMF_CODE.4m.fd \
                /usr/share/ovmf/x64/OVMF_CODE.fd \
                /usr/share/OVMF/OVMF_CODE_4M.fd \
                /usr/share/OVMF/OVMF_CODE.fd \
                /usr/share/ovmf/OVMF.fd))
OVMF_VARS_SRC := $(firstword $(wildcard \
                /usr/share/ovmf/x64/OVMF_VARS.4m.fd \
                /usr/share/ovmf/x64/OVMF_VARS.fd \
                $(subst OVMF_CODE,OVMF_VARS,$(OVMF_CODE))))


# ---------------- kernel flags ----------------
KINC    := -Ikernel/inc -Ilibc/inc -Ibootloader/uefi/inc
KCFLAGS := -ffreestanding -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
           -mcmodel=small -fno-pie -fno-stack-protector \
           -fno-asynchronous-unwind-tables -fno-omit-frame-pointer \
           -O2 -g -Wall -Wextra -MMD -MP $(KINC)
KLDFLAGS := -nostdlib -z max-page-size=0x1000 -T kernel/linker.ld

# ---------------- uefi loader flags ----------------
BCFLAGS := -ffreestanding -m64 -mno-red-zone -DEFI_FUNCTION_WRAPPER \
           -fPIC -fshort-wchar -fno-stack-protector -fno-strict-aliasing \
           -fno-asynchronous-unwind-tables -Wall -MMD -MP \
           -I$(EFIINC) -I$(EFIINC)/x86_64 -Ibootloader/uefi/inc
BLDFLAGS := -nostdlib -shared -Bsymbolic -znocombreloc -T $(EFILDS)

# ---------------- disk layout ----------------
BUILD   := build
DISK    := $(BUILD)/trs.img
DISK_SZ := 256M
ESP_IMG := $(BUILD)/esp.img
ESP_KB  := 262144 
ESP_OFF := 1M

# ---------------- files ----------------
BOOT_ELF  := $(BUILD)/boot.elf
BOOT_EFI  := $(BUILD)/BOOTX64.EFI
KERNEL    := $(BUILD)/kernel.elf
KERNELBIN := $(BUILD)/kernel.bin

KSRC := $(wildcard kernel/*.c kernel/driver/*.c libc/*.c)
KASM := $(wildcard kernel/*.asm)
KOBJ := $(patsubst %.c,$(BUILD)/%.o,$(KSRC)) \
        $(patsubst %.asm,$(BUILD)/%.o,$(KASM))
BOBJ := $(BUILD)/boot/boot.o

DIRS := $(BUILD) $(BUILD)/boot

# ---------------- targets ----------------
.PHONY: all kernel boot disk run debug clean
all: disk

kernel: $(KERNELBIN)
boot:   $(BOOT_EFI)
disk:   $(DISK)

# --- common object rules ---
$(DIRS):
	mkdir -p $@

$(BUILD)/%.o: %.c | $(DIRS)
	@mkdir -p $(dir $@)
	$(CC) $(KCFLAGS) -c $< -o $@

$(BUILD)/%.o: %.asm | $(DIRS)
	@mkdir -p $(dir $@)
	$(NASM) -f elf64 $< -o $@

# --- kernel: ELF -> flat binary ---
# $(KERNEL): $(KOBJ) kernel/linker.ld | $(DIRS)
#	@echo "[kernel] linked: $@"

#$(KERNELBIN): $(KERNEL)
#	$(OBJCOPY) -O binary $< $@
#	@echo "[kernel] flat binary: $@ ($$(stat -c%s $@) bytes)"

# --- uefi boot: ELF -> PE32+ .EFI ---
$(BOBJ): bootloader/uefi/boot.c | $(DIRS)
	@mkdir -p $(dir $@)
	$(CC) $(BCFLAGS) -c $< -o $@

$(BOOT_ELF): $(BOBJ) $(EFICRT0) $(EFILDS) | $(DIRS)
	$(LD) $(BLDFLAGS) $(EFICRT0) $(BOBJ) /usr/lib/libgnuefi.a /usr/lib/libefi.a -o $@

$(BOOT_EFI): $(BOOT_ELF)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .rodata \
           -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* \
           -j .rel* -j .rela* -j .areloc -j .reloc \
           -O efi-app-x86_64 $< $@
	@echo "[boot] $@"

# --- GPT disk with FAT32 ESP ---
$(DISK): $(BOOT_EFI) #$(KERNELBIN)
	@echo "[disk] GPT + FAT32 ESP -> $@"
	rm -f $(ESP_IMG)
	$(MKFAT) -F32 -C $(ESP_IMG) $(ESP_KB)
	$(TRUNC) -s $(DISK_SZ) $(DISK)
	$(PARTED) -s $(DISK) mklabel gpt
	$(PARTED) -s $(DISK) mkpart ESP fat32 1MiB 65MiB
	$(PARTED) -s $(DISK) set 1 esp on
	$(MMD) -i $(ESP_IMG) ::/EFI ::/EFI/BOOT
	$(MCOPY) -i $(ESP_IMG) $(BOOT_EFI) ::/EFI/BOOT/BOOTX64.EFI
#	$(MCOPY) -i $(ESP_IMG) $(KERNELBIN) ::/kernel.bin
	$(DD) if=$(ESP_IMG) of=$(DISK) bs=$(ESP_OFF) seek=1 conv=notrunc status=none
	@echo "[disk] done: $@"

# --- run / debug (OVMF) ---
ifneq ($(strip $(OVMF_CODE)),)
RUN_VARS  := $(BUILD)/OVMF_VARS.fd
$(RUN_VARS): $(OVMF_VARS_SRC) | $(DIRS)
	$(CP) $< $@
RUN_FLAGS := -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
             -drive if=pflash,format=raw,file=$(RUN_VARS)
endif

run: $(DISK) $(RUN_VARS)
	$(QEMU) -m 512M -serial stdio $(RUN_FLAGS) -drive format=raw,media=disk,file=$(DISK)

debug: $(DISK) $(RUN_VARS)
	$(QEMU) -m 512M -serial stdio -s -S $(RUN_FLAGS) -drive format=raw,media=disk,file=$(DISK)

# --- deps & clean ---
#-include $(KOBJ:.o=.d) $(BOBJ:.o=.d)

clean:
	rm -rf $(BUILD)
