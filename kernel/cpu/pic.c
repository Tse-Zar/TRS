#include <pic.h>
#include <io.h>

void pic_remap(uchar off) {
    uint flags = irq_save();

    outb(PIC1_CMD, 0x11);     io_wait();
    outb(PIC2_CMD, 0x11);     io_wait();
    outb(PIC1_DATA, off);     io_wait();
    outb(PIC2_DATA, off + 8); io_wait();
    outb(PIC1_DATA, 0x04);    io_wait();
    outb(PIC2_DATA, 0x02);    io_wait();
    outb(PIC1_DATA, 0x01);    io_wait();
    outb(PIC2_DATA, 0x01);    io_wait();

    outb(PIC1_DATA, 0xFB);
    outb(PIC2_DATA, 0xFF);

    irq_restore(flags);
}

void pic_unmask(uchar line) {
    ushort port = (line < 8) ? PIC1_DATA : PIC2_DATA;
    uchar bit = (uchar)(1 << (line & 7));
    outb(port, (uchar)(inb(port) & ~bit));
}

void pic_mask(uchar line) {
    ushort port = (line < 8) ? PIC1_DATA : PIC2_DATA;
    uchar bit = (uchar)(1 << (line & 7));
    outb(port, (uchar)(inb(port) | bit));
}
