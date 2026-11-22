#ifndef PIC_H
#define PIC_H

#include <types.h>

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI   0x20

#define IRQ_KBD_LINE 1

void pic_remap(uchar offset);
void pic_unmask(uchar irq_line);
void pic_mask(uchar irq_line);

#endif // PIC_H
