#include <pic.h>
#include <io.h>

void pic_remap(unsigned char off) {
    unsigned flags = irq_save();
    
    outb(PIC1_CMD, 0x11);     io_wait();
    outb(PIC2_CMD, 0x11);     io_wait();
    outb(PIC1_DATA, off);     io_wait();
    outb(PIC2_DATA, off + 8); io_wait();
    outb(PIC1_DATA, 0x04);    io_wait();
    outb(PIC2_DATA, 0x02);    io_wait();
    outb(PIC1_DATA, 0x01);    io_wait();
    outb(PIC2_DATA, 0x01);    io_wait();

    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    irq_restore(flags);
}

void pic_unmask(unsigned char line) {
    unsigned short port = (line < 8) ? PIC1_DATA : PIC2_DATA;
    unsigned char bit = (unsigned char)(1 << (line & 7));
    outb(port, (unsigned char)(inb(port) & ~bit)); 
}

void pic_mask(unsigned char line) {
    unsigned short port = (line < 8) ? PIC1_DATA : PIC2_DATA;
    unsigned char bit = (unsigned char)(1 << (line & 7));
    outb(port, (unsigned char)(inb(port) | bit));
}