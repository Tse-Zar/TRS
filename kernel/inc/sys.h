/*
    syscalls
*/

#ifndef SYS_H
#define SYS_H

#include <idt.h>

#define SYSCALL_VECTOR 0x77


#define SYS_WRITE     1
#define SYS_READ      2
#define SYS_CLI_CLEAR 3
#define SYS_SET_COL   4
#define SYS_SET_CRSR  5
#define SYS_OPENDIR   6
#define SYS_CLOSEDIR  7
#define SYS_READDIR   8


void syscall_init(void);
void syscall_handler(_regs_t* reg);

#endif // SYS_H