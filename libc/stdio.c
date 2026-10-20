#include <stdio.h>
#include <syscalls.h>

void printf(const char *str) {
    syscall1(SYS_WRITE, (void*)str);
}