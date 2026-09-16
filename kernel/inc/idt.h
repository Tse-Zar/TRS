#ifndef IDT_H
#define IDT_H

#define IRQ_BASE    0x20
#define IRQ_KBD     0x20

typedef void (*isr_handler_t)(void);

void idt_init(void);
void irq_install(unsigned char irq, isr_handler_t handler);

#endif // IDT_H