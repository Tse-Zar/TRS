#include <stdio.h>
#include <syscall.h>

void printf(const char *str) {
    syscall1(SYS_WRITE, (void*)str);
}

unsigned int scanf(char *buf, unsigned long long len) {
    return syscall2(SYS_READ, buf, (void*)len);
}