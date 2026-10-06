#ifndef CPU_H
#define CPU_H

typedef struct {
    unsigned short limit_low;
    unsigned short base_low;
    unsigned short base_mid;
    unsigned char access;
    unsigned char gran;
    unsigned char base_hight;
} __attribute__((packed)) gdt_entry;

typedef struct {
    unsigned short lim;
    unsigned long long base;
} __attribute__((packed)) gdt_ptr;

void gdt_init(void);
extern void gdt_flush(unsigned long long gdtr_addr);

#endif // CPU_H