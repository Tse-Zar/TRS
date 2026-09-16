# ============================================================
#  TRS OS — Makefile
#  Собирает: stage1, stage2, kernel (flat binary), образ диска
# ============================================================

AS      = nasm
CC      = gcc
LD      = ld
OBJCOPY = objcopy
LDFLAGS = -m elf_i386 -T kernel/linker.ld -nostdlib

BOOT_DIR  = bootloader
BUILD_DIR = build
$(shell mkdir -p $(BUILD_DIR))

# Автоматически собираем все директории **/inc
INC_DIRS := $(shell find . -type d -name inc)
INCLUDES = $(addprefix -I,$(INC_DIRS))

# --- Лимиты, зашитые в бутлоадер (держим в синхроне с .asm!) ---
STAGE1_MAX  = 512            # 1 сектор
STAGE2_MAX  = 64 * 512       # 64 сектора (см. boot_one.asm STAGE2_SECTORS)
KERNEL_MAX  = 256 *512      # 256 секторов (см. boot_two.asm KERNEL_SECTORS)

CFLAGS = -m32 -ffreestanding -fno-pie -nostdlib -fno-stack-protector \
         -fno-asynchronous-unwind-tables -fno-builtin -O2 -Wall -Wextra \
         -fno-strict-aliasing -mgeneral-regs-only -MMD -MP $(INCLUDES)

KERNEL     = $(BUILD_DIR)/kernel.elf
KERNEL_BIN = $(BUILD_DIR)/kernel_flat.bin

# --- Автосбор всех .c ядра, libc и trfs ---
KERNEL_SRCS := $(shell find kernel libc trfs -name '*.c')
KERNEL_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(KERNEL_SRCS))
ENTRY_OBJ   := $(BUILD_DIR)/kernel/entry.o
DEPS        := $(KERNEL_OBJS:.o=.d)

.PHONY: all clean run debug

all: $(BUILD_DIR)/disk.img

$(BUILD_DIR)/stage1.bin: $(BOOT_DIR)/boot_one.asm
	$(AS) -f bin $< -o $@

$(BUILD_DIR)/stage2.bin: $(BOOT_DIR)/boot_two.asm
	$(AS) -f bin $< -o $@
	
# --- Компиляция C: одно правило на все файлы ---
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/entry.o: kernel/entry.asm | $(BUILD_DIR)
	@mkdir -p $(dir $@)
	$(AS) -f elf32 $< -o $@

# --- Link kernel → ELF ---
$(KERNEL): $(ENTRY_OBJ) $(KERNEL_OBJS) kernel/linker.ld | $(BUILD_DIR)
	$(LD) $(LDFLAGS) $(ENTRY_OBJ) $(KERNEL_OBJS) -o $@

# --- ELF → flat binary + проверка лимита ---
$(KERNEL_BIN): $(KERNEL)
	$(OBJCOPY) -O binary $< $@
	@stat -c%s $@ | awk -v m=$(KERNEL_MAX) '{ if ($$1 > m) { print "ERROR: kernel > 256 sectors, increase KERNEL_SECTORS"; exit 1 } }'

# --- Disk image ---
# Layout: sector 0 = stage1, sectors 1-64 = stage2, sectors 65+ = kernel (flat)
$(BUILD_DIR)/disk.img: $(BUILD_DIR)/stage1.bin $(BUILD_DIR)/stage2.bin $(KERNEL_BIN)
	dd if=/dev/zero of=$@ bs=512 count=2048
	dd if=$(BUILD_DIR)/stage1.bin of=$@ bs=512 conv=notrunc
	dd if=$(BUILD_DIR)/stage2.bin of=$@ bs=512 seek=1 conv=notrunc
	dd if=$(KERNEL_BIN) of=$@ bs=512 seek=65 conv=notrunc

# --- Run in QEMU ---
run: all
	qemu-system-i386 -drive format=raw,file=$(BUILD_DIR)/disk.img

# --- Отладка: логируем исключения CPU и сбросы ---
debug: all
	qemu-system-i386 -drive format=raw,file=$(BUILD_DIR)/disk.img \
		-d int,cpu_reset -no-reboot

clean:
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(DEPS)