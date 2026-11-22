#include <stdio.h>
#include <syscall.h>

void printf(const char *str) {
    syscall1(SYS_WRITE, (void*)str);
}

uint scanf(char *buf, ullong len) {
    return syscall2(SYS_READ, buf, (void*)len);
}

void cli_clear(void) {
    syscall1(SYS_CLI_CLEAR, 0);
}

void set_color(ullong fg, ullong bg) {
    syscall2(SYS_SET_COL, fg ? (void*)fg : 0, bg ? (void*)bg : 0);
}

void set_crsr(ullong x, ullong y) {
    syscall2(SYS_SET_CRSR, x ? (void*)x : 0, y ? (void*)y : 0);
}

void delay_ms(ullong ms) {
    syscall1(SYS_DELAY, (void*)ms);
}

void flip(void) {
    syscall1(SYS_FRAME_FLIP, 0);
}
