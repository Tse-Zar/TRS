#include <stdio.h>
#include <syscall.h>

void printf(const char *str) {
    syscall1(SYS_WRITE, (void*)str);
}

unsigned int scanf(char *buf, unsigned long long len) {
    return syscall2(SYS_READ, buf, (void*)len);
}

void cli_clear(void) {
    syscall1(SYS_CLI_CLEAR, 0);
}

void set_color(unsigned long long fg, unsigned long long bg) {
    syscall2(SYS_SET_COL, fg ? (void*)fg : 0, bg ? (void*)bg : 0);
}

void set_crsr(unsigned long long x, unsigned long long y) {
    syscall2(SYS_SET_CRSR, x ? (void*)x : 0, y ? (void*)y : 0);
}