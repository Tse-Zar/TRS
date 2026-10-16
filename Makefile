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

# ---------------- gnu-efi (auto-detect: Arch/Debian -> /usr/lib, Fedora -> /usr/lib64/gnuefi) ----------------
GNUEFI_DIRS ?= /usr/lib /usr/lib64 /usr/lib/gnuefi /usr/lib64/gnuefi
EFICRT0 := $(firstword $(wildcard $(addsuffix /crt0-efi-x86_64.o,$(GNUEFI_DIRS))))
EFILDS  := $(firstword $(wildcard $(addsuffix /elf_x86_64_efi.lds,$(GNUEFI_DIRS))))
EFILIBS := $(firstword $(wildcard $(addsuffix /libgnuefi.a,$(GNUEFI_DIRS)))) \
           $(firstword $(wildcard $(addsuffix /libefi.a,$(GNUEFI_DIRS))))
EFIINC  ?= $(firstword $(wildcard /usr/include/efi))

# gnu-efi is needed only for the boot loader; 'make clean' / 'make kernel' work without it
GOALS := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),all)
ifneq ($(filter all boot disk run debug,$(GOALS)),)
ifeq ($(and $(EFICRT0),$(EFILDS),$(EFIINC)),)
$(error gnu-efi not found: install the 'gnu-efi' package (crt0-efi-x86_64.o, elf_x86_64_efi.lds, headers))
endif
endif

# ---------------- ovmf (make run) ----------------
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
KINC    := -Ikernel/inc -Ilibc/inc -Ibootloader/uefi/inc -Itrfs/fat32
KCFLAGS := -ffreestanding -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
           -mcmodel=small -fno-pie -fno-stack-protector \
           -fno-asynchronous-unwind-tables -fno-omit-frame-pointer \
           -fcf-protection=none \
           -O2 -g -Wall -Wextra -MMD -MP $(KINC)
KLDFLAGS := -nostdlib -z max-page-size=0x1000 -z noexecstack -T kernel/linker.ld

# ---------------- uefi loader flags ----------------
BCFLAGS := -ffreestanding -m64 -mno-red-zone -DEFI_FUNCTION_WRAPPER \
           -fPIC -fshort-wchar -fno-stack-protector -fno-strict-aliasing \
           -fcf-protection=none \
           -fno-asynchronous-unwind-tables -Wall -MMD -MP \
           -I$(EFIINC) -I$(EFIINC)/x86_64 -Ibootloader/uefi/inc
BLDFLAGS := -nostdlib -shared -Bsymbolic -znocombreloc -z noexecstack -T $(EFILDS)

# ---------------- disk layout ----------------
BUILD   := build
DISK    := $(BUILD)/trs.img
DISK_SZ := 64M
ESP_IMG := $(BUILD)/esp.img
ESP_KB  := 63488   # 62 MiB - exactly the 1MiB..63MiB ESP partition
ESP_OFF := 1M

# ---------------- files ----------------
BOOT_ELF  := $(BUILD)/boot.elf
BOOT_EFI  := $(BUILD)/BOOTX64.EFI
KERNEL    := $(BUILD)/kernel.elf
KERNELBIN := $(BUILD)/kernel.bin
FONT_PSF := ter-v32b.psf
FONT_OBJ := $(BUILD)/terminus.o

KSRC := $(wildcard kernel/*.c kernel/driver/*.c libc/*.c kernel/cpu/*.c trfs/fat32/*.c)
KASM := $(wildcard kernel/*.asm kernel/cpu/*.asm)

KOBJ := $(patsubst %.c,$(BUILD)/%.o,$(KSRC)) \
        $(patsubst %.asm,$(BUILD)/%.o,$(KASM)) \
        $(FONT_OBJ)
BOBJ := $(BUILD)/boot/boot.o

DIRS := $(BUILD) $(BUILD)/boot

# ---------------- targets ----------------
.PHONY: all kernel boot disk run debug clean log
.SECONDARY: $(KOBJ) $(BOBJ)
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

$(FONT_OBJ): $(FONT_PSF) | $(DIRS)
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386 $< $@
	@echo "[font] embedded: $< -> $@"

# --- kernel: ELF -> flat binary ---
$(KERNEL): $(KOBJ) kernel/linker.ld | $(DIRS)
	$(LD) $(KLDFLAGS) $(KOBJ) -o $@
	@echo "[kernel] linked: $@"

$(KERNELBIN): $(KERNEL)
	$(OBJCOPY) -O binary $< $@
	@echo "[kernel] flat binary: $@ ($$(stat -c%s $@) bytes)"

# --- uefi boot: ELF -> PE32+ .EFI ---
$(BOBJ): bootloader/uefi/boot.c | $(DIRS)
	@mkdir -p $(dir $@)
	$(CC) $(BCFLAGS) -c $< -o $@

$(BOOT_ELF): $(BOBJ) $(EFICRT0) $(EFILDS) | $(DIRS)
	$(LD) $(BLDFLAGS) $(EFICRT0) $(BOBJ) $(EFILIBS) -o $@

$(BOOT_EFI): $(BOOT_ELF)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .rodata \
           -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* \
           -j .rel* -j .rela* -j .areloc -j .reloc \
           -O efi-app-x86_64 $< $@
	@echo "[boot] $@"

# --- GPT disk with FAT32 ESP ---
# on-disk tree (matches README):
#   /EFI/BOOT/BOOTX64.EFI
#   /space/sys/kern/kernel.bin
#   /space/user/
#   /dump/
#   /sand/
$(DISK): $(BOOT_EFI) $(KERNELBIN)
	@echo "[disk] GPT + FAT32 ESP -> $@"
	rm -f $(ESP_IMG)
	$(MKFAT) -F32 -C $(ESP_IMG) $(ESP_KB)
	$(TRUNC) -s $(DISK_SZ) $(DISK)
	$(PARTED) -s $(DISK) mklabel gpt
	$(PARTED) -s $(DISK) mkpart ESP fat32 1MiB 63MiB
	$(PARTED) -s $(DISK) set 1 esp on
	$(MMD) -i $(ESP_IMG) ::/EFI ::/space ::/dump ::/sand
	$(MMD) -i $(ESP_IMG) ::/EFI/BOOT ::/space/sys ::/space/user
	$(MMD) -i $(ESP_IMG) ::/space/sys/kern
	$(MCOPY) -i $(ESP_IMG) $(BOOT_EFI) ::/EFI/BOOT/BOOTX64.EFI
	$(MCOPY) -i $(ESP_IMG) $(KERNELBIN) ::/space/sys/kern/kernel.bin
	$(DD) if=$(ESP_IMG) of=$(DISK) bs=$(ESP_OFF) seek=1 conv=notrunc status=none
	@echo "[disk] done: $@"

# --- run / debug (OVMF) ---
ifeq ($(strip $(OVMF_CODE)),)
run debug:
	$(error OVMF firmware not found: install the 'ovmf' (Arch/Debian) or 'edk2-ovmf' (Fedora) package)
else
RUN_VARS  := $(BUILD)/OVMF_VARS.fd
$(RUN_VARS): $(OVMF_VARS_SRC) | $(DIRS)
	$(CP) $< $@

RUN_FLAGS := -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
             -drive if=pflash,format=raw,file=$(RUN_VARS)

run: $(DISK) $(RUN_VARS)
	$(QEMU) -m 512M -vga std -global VGA.xres=1280 -global VGA.yres=720 -display gtk,zoom-to-fit=on $(RUN_FLAGS) -drive format=raw,media=disk,file=$(DISK)

log: $(DISK) $(RUN_VARS)
	GDK_BACKEND=x11 $(QEMU) -m 512M -serial file:qemu.log -no-reboot -no-shutdown -d int,cpu_reset -D qemu.log -display gtk,zoom-to-fit=off $(RUN_FLAGS) -drive format=raw,media=disk,file=$(DISK)

debug: $(DISK) $(RUN_VARS)
	GDK_BACKEND=x11 $(QEMU) -m 512M -serial file:qemu.log -no-reboot -no-shutdown -d int,cpu_reset -D qemu.log -s -S -display gtk,zoom-to-fit=off $(RUN_FLAGS) -drive format=raw,media=disk,file=$(DISK)
endif

# --- deps & clean ---
-include $(KOBJ:.o=.d) $(BOBJ:.o=.d)

clean:
	rm -rf $(BUILD)
