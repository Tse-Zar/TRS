#include <cpu.h>

gdt_entry gdt[5]; // NULL - kernel CODE - kernel DATA - user CODE - user DATA
gdt_ptr gp;

static void gdt_set_gate(int num, unsigned char access, unsigned char gran) {
    gdt[num].base_low   = 0;
    gdt[num].limit_low  = 0;
    gdt[num].base_hight = 0;
    gdt[num].base_mid   = 0;

    gdt[num].access = access;
    gdt[num].gran   = (gran >> 16) & 0xF0;
}

void gdt_init(void) {
    gp.lim  = sizeof(gdt) - 1;
    gp.base = (unsigned long long)&gdt;

    gdt_set_gate(0, 0, 0);
    gdt_set_gate(1, 0x9A, 0x20);
    gdt_set_gate(2, 0x92, 0x00);
    gdt_set_gate(3, 0xFA, 0x20);
    gdt_set_gate(4, 0xF2, 0x00);

    gdt_flush((unsigned long long)&gp);
}

