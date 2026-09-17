/*
    ===============================
    = TSEZAR TSEZAR TSEZAR TSEZAR =
    = --------------------------- =
    = Interrupt Descriptor Table  =
    ===============================
*/

#ifndef IDT_H
#define IDT_H

#define IRQ_BASE    0x20

typedef void (*isr_handler_t)(void);

typedef struct {
    unsigned gs, fs, es, ds;
    unsigned edi, esi, ebp, esp0, ebx, edx, ecx, eax;
    unsigned int_no, err_code;
    unsigned eip, cs, eflags;
} _regs_t;

void idt_init(void);
void irq_install(unsigned char irq, isr_handler_t handler);
void isr_install(unsigned char vector, isr_handler_t handler);
void isr_dispatcher(_regs_t* reg);

#endif // IDT_H