/*
    ===============================
    = TSEZAR TSEZAR TSEZAR TSEZAR =
    = --------------------------- =
    = Interrupt Descriptor Table  =
    ===============================
*/

#include <idt.h>
#include <pic.h>
#include <string.h>
#include <io.h>
#include <gdt.h>
#include <panic.h>

#define IDT_ENTRIES     256
#define IDT_FLAGS_RINGO 0x8E

typedef struct {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char  zero;
    unsigned char  flags;
    unsigned short offset_mid;
    unsigned int   offset_high;
    unsigned int   reserved;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    unsigned short limit;
    unsigned long  base;
} __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idt_ptr;

static isr_handler_t isr_handlers[256];
static isr_handler_t irq_handlers[16];

extern void* isr_stub_table[256];

static const char* const exc_names[32] = {
    "Division by zero", "Debug", "NMI", "Breakpoint",
    "Overflow", "Bound range exceeded", "Invalid opcode", "No FPU",
    "Double fault", "Coprocessor segment", "Invalid TSS", "Segment not present",
    "Stack fault", "General protection fault", "Page fault", "Reserved",
    "x87 exception", "Alignment check", "Machine check", "SIMD exception",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved"
};

static void idt_set_gate(unsigned char num, void* handler, unsigned char ist) {
    unsigned long addr = (unsigned long)handler;

    idt[num].offset_low  = (unsigned short)(addr & 0xFFFF);
    idt[num].offset_mid  = (unsigned short)((addr >> 16) & 0xFFFF);
    idt[num].offset_high = (unsigned int)((addr >> 32) & 0xFFFFFFFF);
    idt[num].selector    = GDT_KERNEL_CODE;
    idt[num].zero        = ist & 0x0F;
    idt[num].flags       = IDT_FLAGS_RINGO ;
    idt[num].reserved    = 0;
}

void isr_install(unsigned char vector, isr_handler_t handler) {
    isr_handlers[vector] = handler;
}

void irq_install(unsigned char irq, isr_handler_t handler) {
    if(irq < 16) irq_handlers[irq] = handler;
}

static int irq_is_spurious(unsigned irq) {
    unsigned short cmd = (irq < 8) ? PIC1_CMD : PIC2_CMD;
    outb(cmd, 0x0B);
    return !(inb(cmd) & (1 << (irq & 7)));
}

void isr_dispatcher(_regs_t *reg) {
    if(isr_handlers[reg->int_no]) isr_handlers[reg->int_no](reg);

    if(reg->int_no < 32) {

        kpanic_regs(reg, exc_names[reg->int_no]);
        return;
    }

    if(reg->int_no < IRQ_BASE || reg->int_no >= IRQ_BASE + 16) {
        return;
    }

    unsigned irq = reg->int_no - IRQ_BASE;
    if(irq == 7 || irq == 15) {
        if(irq_is_spurious(irq)) {
            if(irq == 15) outb(PIC1_CMD, PIC_EOI);
            return;
        }
    }

    if(irq_handlers[irq]) irq_handlers[irq](reg);

    if(irq >= 8) outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}

void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    memset((void*)isr_handlers, 0, sizeof(isr_handlers));
    memset((void*)irq_handlers, 0, sizeof(irq_handlers));

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (unsigned long)idt;

    for(unsigned i = 0; i < 256; ++i) {
        idt_set_gate((unsigned char)i, isr_stub_table[i], (i == 8) ? 1 : 0);
    }

    __asm__ volatile("lidt %0" :: "m"(idt_ptr));
}