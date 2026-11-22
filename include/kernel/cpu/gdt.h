/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = Global Description Table for kern  =
    = Modes: NULL, Kernel Code, Kernel   =
    = Data, User Code, User Data ------- =
    ======================================
*/

#ifndef GDT_H
#define GDT_H

#include <types.h>

#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE   0x20
#define GDT_USER_DATA   0x18
#define GDT_TSS         0x28

typedef struct {
    ushort limit_low;
    ushort base_low;
    uchar  base_mid;
    uchar  access;
    uchar  flags;
    uchar  base_high;
} __attribute__((packed)) gdt_entry;

typedef struct {
    ushort limit;
    ullong base;
} __attribute__((packed)) gdt_ptr;

void gdt_init(void);
void tss_set_kernel_stack(ullong rsp0);

#endif
