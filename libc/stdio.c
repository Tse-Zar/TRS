#include <stdio.h>
#include <syscall.h>

void printf(const char *str) {
    syscall1(SYS_WRITE, (void*)str);
}