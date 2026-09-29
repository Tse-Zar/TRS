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
    unsigned long gs, fs, es, ds;
    unsigned long r8, r9, r10, r11, r12, r13, r14, r15;
    unsigned long rdi, rsi, rbp, rbx, rdx, rcx, rax;
    unsigned long int_no, err_code;
    unsigned long rip, cs, rflags, rsp, ss;
} _regs_t;

void idt_init(void);
void irq_install(unsigned char irq, isr_handler_t handler);
void isr_install(unsigned char vector, isr_handler_t handler);
void isr_dispatcher(_regs_t* reg);

#endif // IDT_H