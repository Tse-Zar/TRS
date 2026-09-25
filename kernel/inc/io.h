#ifndef IO_H
#define IO_H

static inline unsigned char inb(unsigned short port) {
    unsigned char byte;
    __asm__ volatile("inb %1, %0" : "=a"(byte) : "Nd"(port));

    return byte;
}

static inline void outb(unsigned short port, unsigned char byte) {
    __asm__ volatile ("outb %0, %1" :: "a"(byte), "Nd"(port));
}

static inline void io_wait(void) { outb(0x80, 0); }

static inline unsigned irq_save(void) {
    unsigned flags;
    __asm__ volatile("pushfq; popq %0; cli" : "=r"(flags) :: "memory");
    return flags;
}

static inline void irq_restore(unsigned flags) {
    __asm__ volatile("pushq %0; popfq" :: "r"(flags) : "memory");
}

#endif //IO_H