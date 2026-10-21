#include <syscall.h>

unsigned int syscall1(int call, void *one) {
    long long ret;
    __asm__ volatile ("int $0x77\n\t"
                      : "=a"(ret)
                      : "a"(call), "d"(one)
                      : "memory");
    return  ret;
}

unsigned int syscall2(int call, void* one, void* two) {
    long long ret;
    __asm__ volatile ("int $0x77\n\t"
                      : "=a"(ret)
                      : "a"(call), "d"(one), "D"(two)
                      : "memory");
    
    return ret;
}