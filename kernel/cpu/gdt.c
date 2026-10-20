#include <gdt.h>
#include <string.h>

#define GDT_ENTRIES 7

typedef struct {
    unsigned int       reserved0;
    unsigned long long rsp0, rsp1, rsp2;
    unsigned long long reserved1;
    unsigned long long ist[7];
    unsigned long long reserved2;
    unsigned short     reserved3;
    unsigned short     iomap_base;
} __attribute__((packed)) tss_t;

static gdt_entry gdt[GDT_ENTRIES + 1];
static gdt_ptr   gdt_p;
static tss_t     tss;

static unsigned char df_stack[8192] __attribute__((aligned(16)));

static void gdt_set_gate(int i, unsigned int base, unsigned int limit, unsigned char access,
                         unsigned char flags) {
    gdt[i].limit_low = (unsigned short)(limit & 0xFFFF);
    gdt[i].base_low  = (unsigned short)(base & 0xFFFF);
    gdt[i].base_mid  = (unsigned char)((base >> 16) & 0x0FF);
    gdt[i].access    = access;
    gdt[i].flags     = (unsigned char)(((limit >> 16) & 0x0F) | (flags & 0xF0));
    gdt[i].base_high = (unsigned char)((base >> 24) & 0xFF);
}

static void gdt_set_tss(int i) {
    unsigned long long base = (unsigned long long)&tss;
    unsigned int limit      = sizeof(tss) - 1;

    gdt[i].limit_low = (unsigned short)(limit & 0xFFFF);
    gdt[i].base_low  = (unsigned short)(base & 0xFFFF);
    gdt[i].base_mid  = (unsigned char)((base >> 16) & 0xFF);
    gdt[i].access    = 0x89; 
    gdt[i].flags     = (unsigned char)((limit >> 16) & 0x0F);
    gdt[i].base_high = (unsigned char)((base >> 24) & 0xFF);

    unsigned int* upper_slot = (unsigned int*)&gdt[i + 1];
    upper_slot[0] = (unsigned int)(base >> 32);
    upper_slot[1] = 0;
}


void tss_set_kernel_stack(unsigned long long rsp0) { tss.rsp0 = rsp0; }

void gdt_init(void) {
    memset(gdt, 0, sizeof(gdt));
    memset(&tss, 0, sizeof(tss));
    tss.iomap_base = sizeof(tss);
    tss.ist[0] = (unsigned long long)df_stack + sizeof(df_stack);

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFF, 0x9A, 0xA0);
    gdt_set_gate(2, 0, 0xFFFFFF, 0x92, 0xC0);
    gdt_set_gate(3, 0, 0xFFFFFF, 0xFA, 0xA0);  // user code, DPL=3
    gdt_set_gate(4, 0, 0xFFFFFF, 0xF2, 0xC0);  // user data, DPL=3
    gdt_set_tss(5);

    gdt_p.limit = sizeof(gdt) - 1;
    gdt_p.base  = (unsigned long long)&gdt;

    __asm__ volatile (
        "lgdt %0\n\t"
        "pushq %1\n\t"
        "lea 1f(%%rip), %%rax\n\t"
        "pushq %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        "mov %2, %%eax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%ss\n\t"
        "xor %%eax, %%eax\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        :: "m"(gdt_p), "i"(GDT_KERNEL_CODE), "i"(GDT_KERNEL_DATA)
        : "rax", "memory"
    );

    __asm__ volatile("ltr %0" :: "rm"((unsigned short)GDT_TSS));
}