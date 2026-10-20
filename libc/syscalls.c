#include <syscalls.h>

void syscall1(int call, void *one) {
    __asm__ volatile ("int $0x77\n\t"
                      :
                      : "a"(call), "d"(one)
                      : "memory");
}
