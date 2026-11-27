# =============================================================
# = TRS - Tsezar Reliable System | build system               =
# =  build/k/kernel.bin   - kernel flat binary                =
# =  build/a/term.pro     - terminal app (.pro: hdr + image)  =
# =  build/a/univ.pro     - universal app                     =
# =  build/BOOTX64.EFI    - UEFI loader (GNU-EFI, PE32+)      =
# =  build/trs.img        - GPT disk + FAT32 ESP              =
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

# ---------------- gnu-efi (loader only) ----------------
GNUEFI_DIRS ?= /usr/lib /usr/lib64 /usr/lib/gnuefi /usr/lib64/gnuefi
EFICRT0 := $(firstword $(wildcard $(addsuffix /crt0-efi-x86_64.o,$(GNUEFI_DIRS))))
EFILDS  := $(firstword $(wildcard $(addsuffix /elf_x86_64_efi.lds,$(GNUEFI_DIRS))))
EFILIBS := $(firstword $(wildcard $(addsuffix /libgnuefi.a,$(GNUEFI_DIRS)))) \
           $(firstword $(wildcard $(addsuffix /libefi.a,$(GNUEFI_DIRS))))
EFIINC ?= $(firstword $(wildcard /usr/include/efi))

GOALS := $(if $(MAKECMDGOALS),$(MAKECMDGOALS),all)
ifneq ($(filter all boot disk run debug log,$(GOALS)),)
ifeq ($(and $(EFICRT0),$(EFILDS),$(EFIINC)),)
$(error gnu-efi not found: install the 'gnu-efi' package)
endif
endif

# ---------------- OVMF (make run) ----------------
OVMF_CODE := $(firstword $(wildcard \
    /usr/share/ovmf/x64/OVMF_CODE.4m.fd \
    /usr/share/ovmf/x64/OVMF_CODE.fd \
    /usr/share/OVMF/OVMF_CODE_4M.fd \
    /usr/share/OVMF/OVMF_CODE.fd \
    /usr/share/ovmf/OVMF.fd))
OVMF_VARS_SRC := $(firstword $(wildcard \
    /usr/share/ovmf/x64/OVMF_VARS.4m.fd \
    /usr/share/ovmf/OVMF_VARS.fd \
    $(patsubst %_CODE%.fd,%_VARS.fd,$(OVMF_CODE))))

# ---------------- hardening ----------------
HARDEN_CORE := -fstack-protector-strong -mstack-protector-guard=global \
               -fstack-clash-protection -fno-common -fno-omit-frame-pointer

WARN := -Wall -Wextra -Wformat=2 -Wshadow -Wpointer-arith

# ---------------- include dirs (auto-discover) ----------------
INCLUDE_DIRS := $(sort $(patsubst %/,%, $(dir $(shell find include -name '*.h'))))

KINC := -Iinclude -Ikernel $(patsubst %,-I%,$(filter-out include,$(INCLUDE_DIRS)))
TINC := -Iinclude $(patsubst %,-I%,$(filter-out include,$(INCLUDE_DIRS)))

# ---------------- kernel flags ----------------
KCFLAGS := -std=c11 -ffreestanding -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
           -fno-builtin -fno-builtin-memcpy -fno-builtin-memset \
           -mcmodel=kernel -fno-pie \
           -fno-asynchronous-unwind-tables \
           -fcf-protection=none $(HARDEN_CORE) \
           -O3 -g $(WARN) -MMD -MP $(KINC)

KCFLAGS_NOSSP := $(filter-out -fstack-protector-strong -fstack-clash-protection,$(KCFLAGS)) \
                  -fno-stack-protector

KLDFLAGS := -nostdlib -z max-page-size=0x1000 -z noexecstack -T kernel/linker.ld

# ---------------- application flags ----------------
TCFLAGS := -std=c11 -ffreestanding -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
           -mcmodel=small -fPIE \
           -fno-asynchronous-unwind-tables \
           -fcf-protection=none $(HARDEN_CORE) \
           -O2 -g $(WARN) -MMD -MP $(TINC)
ifeq ($(FORTIFY),1)
TCFLAGS += -D_FORTIFY_SOURCE=2
endif
TLDFLAGS := -nostdlib -static -pie -z noexecstack -T programs/terminal/linker.ld
CARLDFLAGS := -nostdlib -static -pie -z noexecstack -T programs/univ/linker.ld

# ---------------- uefi loader flags ----------------
BCFLAGS := -ffreestanding -m64 -mno-red-zone -DEFI_FUNCTION_WRAPPER \
           -fPIC -fshort-wchar -fno-stack-protector -fno-strict-aliasing \
           -fcf-protection=none \
           -fno-asynchronous-unwind-tables -Wall -MMD -MP \
           -I$(EFIINC) -I$(EFIINC)/x86_64 -Iinclude/boot -Iinclude
BLDFLAGS := -nostdlib -shared -Bsymbolic -znocombreloc -z noexecstack -T $(EFILDS)

# ---------------- directories ----------------
KBUILD := build/k
ABUILD := build/a
BUILD  := build

DIRS := $(KBUILD) $(KBUILD)/boot \
        $(KBUILD)/kernel $(KBUILD)/kernel/cpu $(KBUILD)/kernel/proc $(KBUILD)/kernel/mem \
        $(KBUILD)/driver $(KBUILD)/driver/trfs/fat32 \
        $(KBUILD)/libc \
        $(ABUILD) $(ABUILD)/programs/terminal $(ABUILD)/programs/univ \
        $(ABUILD)/libc

# ---------------- source lists ----------------
K_SRC := $(wildcard kernel/kernel.c) \
         $(wildcard kernel/cpu/*.c) \
         $(wildcard kernel/proc/*.c) \
         $(wildcard kernel/mem/*.c) \
         $(wildcard driver/disk.c) \
         $(wildcard driver/gpt.c) \
         $(wildcard driver/image.c) \
         $(wildcard driver/keyboard.c) \
         $(wildcard driver/fat32.c) \
         $(wildcard driver/trfs/fat32/*.c)

K_ASM := $(wildcard kernel/entry.asm) \
         $(wildcard kernel/cpu/*.asm) \
         $(wildcard kernel/proc/syscall.asm)

COMMON_LIBC := $(wildcard libc/string.c) \
               $(wildcard libc/mem.c) \
               $(wildcard libc/mutex.c)

APP_LIBC := $(wildcard libc/stdio.c) \
            $(wildcard libc/syscall.c) \
            $(wildcard libc/dirent.c) \
            $(wildcard libc/stdlib.c) \
            $(wildcard libc/security.c)

BOOT_SRC := bootloader/uefi/boot.c
BOOT_ASM := bootloader/uefi/tramp.asm

TERM_SRC := programs/terminal/tmain.c programs/terminal/commands.c
TERM_ASM := programs/terminal/entry.asm

UNIV_SRC := programs/univ/main.c
UNIV_ASM := programs/univ/univ.asm

FONT_PSF := ter-v32b.psf

# ---------------- object lists ----------------
KOBJ := $(patsubst %.c,$(KBUILD)/%.o,$(K_SRC)) \
        $(patsubst %.asm,$(KBUILD)/%.o,$(K_ASM)) \
        $(patsubst libc/%.c,$(KBUILD)/libc/%.o,$(COMMON_LIBC)) \
        $(KBUILD)/terminus.o

BOOT_OBJ := $(KBUILD)/boot/boot.o $(KBUILD)/boot/tramp.o

TERM_OBJ := $(patsubst %.c,$(ABUILD)/%.o,$(TERM_SRC)) \
            $(patsubst %.asm,$(ABUILD)/%.o,$(TERM_ASM)) \
            $(patsubst libc/%.c,$(ABUILD)/libc/%.o,$(COMMON_LIBC)) \
            $(patsubst libc/%.c,$(ABUILD)/libc/%.o,$(APP_LIBC))

UNIV_OBJ := $(patsubst %.c,$(ABUILD)/%.o,$(UNIV_SRC)) \
            $(patsubst %.asm,$(ABUILD)/%.o,$(UNIV_ASM)) \
            $(patsubst libc/%.c,$(ABUILD)/libc/%.o,$(COMMON_LIBC)) \
            $(patsubst libc/%.c,$(ABUILD)/libc/%.o,$(APP_LIBC))

# ---------------- output files ----------------
BOOT_EFI  := $(BUILD)/BOOTX64.EFI
BOOT_ELF  := $(KBUILD)/boot.elf
KERN_ELF  := $(KBUILD)/kernel.elf
KERN_BIN  := $(KBUILD)/kernel.bin
TERM_ELF  := $(ABUILD)/term.elf
TERM_PRO  := $(ABUILD)/term.pro
UNIV_ELF  := $(ABUILD)/univ.elf
UNIV_PRO  := $(ABUILD)/univ.pro
DISK      := $(BUILD)/trs.img
ESP_IMG   := $(BUILD)/esp.img
DISK_SZ   := 64M
ESP_KB    := 63488
ESP_OFF   := 1M
RUN_VARS  := $(BUILD)/OVMF_VARS.fd

QEMU_CPU ?= max

# =============================================================
# PHONY
# =============================================================
.PHONY: all kernel boot term univ disk run debug log clean analyze help

.DEFAULT_GOAL := all
all: disk

kernel: $(KERN_BIN)
boot:   $(BOOT_EFI)
term:   $(TERM_PRO)
univ:   $(UNIV_PRO)
disk:   $(DISK)

help:
	@echo "Targets:"
	@echo "  all        - build everything (kernel + loader + apps + disk)"
	@echo "  kernel     - build kernel.bin"
	@echo "  boot       - build BOOTX64.EFI"
	@echo "  term       - build term.pro"
	@echo "  univ       - build univ.pro"
	@echo "  disk       - build trs.img (full disk image)"
	@echo "  run        - boot with QEMU + OVMF"
	@echo "  debug      - QEMU with -s -S (gdb on :1234)"
	@echo "  log        - QEMU with serial log"
	@echo "  clean      - remove build/"
	@echo "  help       - this message"

# =============================================================
# RULES
# =============================================================

$(DIRS):
	mkdir -p $@

# --- kernel: .c → .o (KCFLAGS) ---
$(KBUILD)/%.o: %.c | $(DIRS)
	@mkdir -p $(dir $@)
	$(CC) $(KCFLAGS) -c $< -o $@

# kernel/mem/security.c — defines __stack_chk_fail, no SSP
$(KBUILD)/kernel/mem/security.o: kernel/mem/security.c | $(DIRS)
	@mkdir -p $(dir $@)
	$(CC) $(KCFLAGS_NOSSP) -c $< -o $@

# --- kernel: .asm → .o ---
$(KBUILD)/%.o: %.asm | $(DIRS)
	@mkdir -p $(dir $@)
	$(NASM) -f elf64 $< -o $@

# --- common libc (shared by kernel + apps) ---
$(KBUILD)/libc/%.o: libc/%.c | $(DIRS)
	@mkdir -p $(dir $@)
	$(CC) $(KCFLAGS_NOSSP) -c $< -o $@

# --- bootloader ---
$(KBUILD)/boot/%.o: bootloader/uefi/%.c | $(DIRS)
	@mkdir -p $(dir $@)
	$(CC) $(BCFLAGS) -c $< -o $@

$(KBUILD)/boot/%.o: bootloader/uefi/%.asm | $(DIRS)
	@mkdir -p $(dir $@)
	$(NASM) -f elf64 $< -o $@

$(BOOT_ELF): $(BOOT_OBJ) $(EFICRT0) | $(DIRS)
	$(LD) $(BLDFLAGS) $^ $(EFILIBS) -o $@

$(BOOT_EFI): $(BOOT_ELF)
	$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .rodata \
             -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* \
             -j .rel* -j .rela* -j .areloc -j .reloc \
             -O efi-app-x86_64 $< $@
	@echo "[boot] $@"

# --- kernel ELF → flat binary ---
$(KERN_ELF): $(KOBJ) | $(DIRS)
	$(LD) $(KLDFLAGS) $^ -o $@
	@echo "[kernel] linked: $@"

$(KERN_BIN): $(KERN_ELF)
	$(OBJCOPY) -O binary $< $@
	@echo "[kernel] flat binary: $@ ($$(stat -c%s $@) bytes)"

# --- terminal app ---
$(ABUILD)/programs/terminal/%.o: programs/terminal/%.c | $(DIRS)
	@mkdir -p $(dir $@)
	$(CC) $(TCFLAGS) -c $< -o $@

$(ABUILD)/programs/terminal/%.o: programs/terminal/%.asm | $(DIRS)
	@mkdir -p $(dir $@)
	$(NASM) -f elf64 $< -o $@

$(TERM_ELF): $(TERM_OBJ) | $(DIRS)
	$(LD) $(TLDFLAGS) $^ -o $@

$(TERM_PRO): $(TERM_ELF)
	$(OBJCOPY) -O binary --remove-section=.bss $< $@

# --- universal app ---
$(ABUILD)/programs/univ/%.o: programs/univ/%.c | $(DIRS)
	@mkdir -p $(dir $@)
	$(CC) $(TCFLAGS) -c $< -o $@

$(ABUILD)/programs/univ/%.o: programs/univ/%.asm | $(DIRS)
	@mkdir -p $(dir $@)
	$(NASM) -f elf64 $< -o $@

$(UNIV_ELF): $(UNIV_OBJ) | $(DIRS)
	$(LD) $(CARLDFLAGS) $^ -o $@

$(UNIV_PRO): $(UNIV_ELF)
	$(OBJCOPY) -O binary --remove-section=.bss $< $@

# --- font embed ---
$(KBUILD)/terminus.o: $(FONT_PSF) | $(DIRS)
	$(OBJCOPY) -I binary -O elf64-x86-64 -B i386 $< $@
	@echo "[font] embedded: $< -> $@"

# --- app libc ---
$(ABUILD)/libc/%.o: libc/%.c | $(DIRS)
	@mkdir -p $(dir $@)
	$(CC) $(TCFLAGS) -c $< -o $@

# --- disk image ---
$(DISK): $(BOOT_EFI) $(KERN_BIN) $(TERM_PRO) $(UNIV_PRO) | $(DIRS)
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
	$(MCOPY) -i $(ESP_IMG) $(BOOT_EFI)  ::/EFI/BOOT/BOOTX64.EFI
	$(MCOPY) -i $(ESP_IMG) $(KERN_BIN)  ::/space/sys/kern/kernel.bin
	$(MCOPY) -i $(ESP_IMG) $(TERM_PRO)  ::/space/sys/term.pro
	$(MCOPY) -i $(ESP_IMG) $(UNIV_PRO)  ::/space/user/univ.pro
	$(DD) if=$(ESP_IMG) of=$(DISK) bs=$(ESP_OFF) seek=1 conv=notrunc status=none
	@echo "[disk] done: $@"

# --- QEMU ---
ifeq ($(strip $(OVMF_CODE)),)
run debug log:
	$(error OVMF firmware not found: install the 'ovmf' or 'edk2-ovmf' package)
else
$(RUN_VARS): $(OVMF_VARS_SRC) | $(DIRS)
	$(CP) $< $@

RUN_FLAGS := -drive if=pflash,format=raw,readonly=on,file=$(OVMF_CODE) \
              -drive if=pflash,format=raw,file=$(RUN_VARS)

run: $(DISK) $(RUN_VARS)
	$(QEMU) -cpu $(QEMU_CPU) -m 2G -vga std -global VGA.xres=1280 -global VGA.yres=720 \
	        -display gtk,zoom-to-fit=on $(RUN_FLAGS) \
	        -drive format=raw,media=disk,file=$(DISK)

log: $(DISK) $(RUN_VARS)
	GDK_BACKEND=x11 $(QEMU) -m 512M -serial file:serial.log -no-reboot -no-shutdown \
		-d int,cpu_reset -D qemu.log $(RUN_FLAGS) \
		-drive format=raw,media=disk,file=$(DISK)

debug: $(DISK) $(RUN_VARS)
	GDK_BACKEND=x11 $(QEMU) -m 512M -serial file:qemu-serial.log -no-reboot -no-shutdown \
		-d int,cpu_reset -D qemu.log -s -S -display gtk,zoom-to-fit=off $(RUN_FLAGS) \
		-drive format=raw,media=disk,file=$(DISK)
endif

# --- clean ---
clean:
	rm -rf $(BUILD) serial.log qemu.log qemu-serial.log

# --- deps ---
-include $(KOBJ:.o=.d) $(BOOT_OBJ:.o=.d) $(TERM_OBJ:.o=.d) $(UNIV_OBJ:.o=.d)
