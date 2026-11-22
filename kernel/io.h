#ifndef IO_H
#define IO_H

#include <types.h>

static inline uchar inb(ushort port) {
    uchar byte;
    __asm__ volatile("inb %1, %0" : "=a"(byte) : "Nd"(port));

    return byte;
}

static inline void outb(ushort port, uchar byte) {
    __asm__ volatile ("outb %0, %1" :: "a"(byte), "Nd"(port));
}

static inline ushort inw(ushort port) {
    ushort word;
    __asm__ volatile("inw %1, %0" : "=a"(word) : "Nd"(port));

    return word;
}

static inline void outw(ushort w, ushort port) {
    __asm__ volatile("outw %0, %1" :: "a"(w), "Nd"(port));
}

static inline void io_wait(void) { outb(0x80, 0); }

static inline ulong irq_save(void) {
    ulong flags;
    __asm__ volatile("pushfq; popq %0; cli" : "=r"(flags) :: "memory");
    return flags;
}

static inline void irq_restore(ulong flags) {
    __asm__ volatile("pushq %0; popfq" :: "r"(flags) : "memory");
}

#endif //IO_H
