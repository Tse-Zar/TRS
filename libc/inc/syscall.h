#ifndef SYSCALLS_H
#define SYSCALLS_H

#define SYS_WRITE     1
#define SYS_READ      2
#define SYS_CLI_CLEAR 3
#define SYS_SET_COL   4
#define SYS_SET_CRSR  5

unsigned int syscall1(int call, void *one);
unsigned int syscall2(int call, void* one, void* two);

#endif // SYSCALLS_H