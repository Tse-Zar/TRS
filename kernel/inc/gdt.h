/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = Global Description Table for kenr  =
    = Modes: NULL, Kerrel Code, Kernel   =
    = Data, User Code, User Data ------- =
    ======================================
*/

#ifndef GDT_H
#define GDT_H

#define GDT_KERNEL_CODE 0x08
#define GDT_KERNEL_DATA 0x10
#define GDT_USER_CODE   0x18
#define GDT_USER_DATA   0x20
#define GDT_TSS         0x28

typedef struct {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char  base_mid;
    unsigned char  access;
    unsigned char  flags;
    unsigned char  base_high;
} __attribute__((packed)) gdt_entry;

typedef struct {
    unsigned short limit;
    unsigned long long base;
} __attribute__((packed)) gdt_ptr;

void gdt_init(void);
void tss_set_kernel_stack(unsigned long long rsp0);

#endif