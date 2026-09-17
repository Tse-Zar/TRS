/*
    ===============================
    = TSEZAR TSEZAR TSEZAR TSEZAR =
    = --------------------------- =
    = Interrupt Descriptor Table  =
    ===============================
*/

#include <idt.h>
#include <pic.h>
#include <vga.h>
#include <string.h>
#include <io.h>

#define IDR_ENTRIES     256
#define KERNEL_CS       0x08
#define IDT_FLAGS_RINGO 0x8E

typedef struct {
    unsigned short offset_low;
    unsigned short selector;
    unsigned char  zero;
    unsigned char  flags;
    unsigned short  offset_high;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    unsigned short limit;
    unsigned int   base;
} __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[IDR_ENTRIES];
static idt_ptr_t   idt_ptr;

static isr_handler_t isr_handlers[32];
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

static void idt_set_gate(unsigned char num, void* handler) {
    unsigned int addr = (unsigned int)handler;

    idt[num].offset_low = (unsigned short)(addr & 0xFFFF);
    idt[num].offset_high = (unsigned short)(addr >> 16);
    idt[num].selector = KERNEL_CS;
    idt[num].zero = 0;
    idt[num].flags = IDT_FLAGS_RINGO;
}

void isr_install(unsigned char vector, isr_handler_t handler) {
    if(vector < 32) isr_handlers[vector] = handler;
}

void irq_install(unsigned char irq, isr_handler_t handler) {
    if(irq < 16) irq_handlers[irq] = handler;
}

static void exc_panic(_regs_t* r) {
    const char* name = exc_names[r->int_no];

    dsp_set_color(_WHITE, _RED);
    dsp_print("\n\n*** KERNEL PANIC ***\nException: ");
    dsp_print(name);
    dsp_print("\n");

    dsp_print("int=");
    dsp_print_hex(r->int_no);
    dsp_print(" err=");
    dsp_print_hex(r->err_code);
    dsp_print(" eip=");
    dsp_print_hex(r->eip);
    dsp_print(" cs =");
    dsp_print_hex(r->cs);
    dsp_print(" eflags=");
    dsp_print_hex(r->eflags);
    dsp_print("\nSystem halted.\n");

    while(1) __asm__ volatile("cli; hlt"); 
}

void isr_dispatcher(_regs_t *reg) {
    if(reg->int_no < 32) {
        if(isr_handlers[reg->int_no]) {
            isr_handlers[reg->int_no]();
            return;
        }
        exc_panic(reg);
        return;
    }

    unsigned irq = reg->int_no - IRQ_BASE;
    if(irq < 16 && irq_handlers[irq]) {
        irq_handlers[irq]();
    }

    if(irq >= 8) outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}

void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    memset((void*)isr_handlers, 0, sizeof(isr_handlers));
    memset((void*)irq_handlers, 0, sizeof(irq_handlers));

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (unsigned int)idt;

    for(unsigned i = 0; i < 48; ++i) {
        idt_set_gate((unsigned char)i, isr_stub_table[i]);
    }

    __asm__ volatile("lidt %0" :: "m"(idt_ptr));
    __asm__ volatile("sti");
}