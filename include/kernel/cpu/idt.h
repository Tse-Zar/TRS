/*
    ===============================
    = TSEZAR TSEZAR TSEZAR TSEZAR =
    = --------------------------- =
    = Interrupt Descriptor Table  =
    ===============================
*/

#ifndef IDT_H
#define IDT_H

#include <types.h>

#define IRQ_BASE    0x20

typedef struct {
    ulong gs, fs;
    ulong r8, r9, r10, r11, r12, r13, r14, r15;
    ulong rdi, rsi, rbp, rbx, rdx, rcx, rax;
    ulong int_no, err_code;
    ulong rip, cs, rflags, rsp, ss;
} _regs_t;

typedef void (*isr_handler_t)(_regs_t* reg);

void idt_init(void);
void irq_install(uchar irq, isr_handler_t handler);
void isr_install(uchar vector, isr_handler_t handler);
_regs_t* isr_dispatcher(_regs_t* reg);

#endif // IDT_H
