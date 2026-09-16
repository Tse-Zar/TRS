#ifndef PIC_H
#define PIC_H

#define PIC1_CMD  0x20
#define PIC1_DATA 0x21
#define PIC2_CMD  0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI   0x20

#define IRQ_KBD_LINE 1

void pic_remap(unsigned char offset);
void pic_unmask(unsigned char irq_line);
void pic_mask(unsigned char irq_line);

#endif // PIC_H