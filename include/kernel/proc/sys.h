/*
    syscalls
*/

#ifndef SYS_H
#define SYS_H

#include <idt.h>
#include <types.h>

#define SYSCALL_VECTOR 0x77

#define SYS_WRITE       1
#define SYS_READ        2
#define SYS_CLI_CLEAR   3
#define SYS_SET_COL     4
#define SYS_SET_CRSR    5
#define SYS_OPENDIR     6
#define SYS_CLOSEDIR    7
#define SYS_READDIR     8
#define SYS_DELAY       9
#define SYS_START_PROC  10
#define SYS_FRAME_FLIP  11
#define SYS_IPC_SEND    12
#define SYS_IPC_RECV    13
#define SYS_IPC_REPLY   14
#define SYS_IPC_CALL    15
#define SYS_IPC_MAKE_EP 16
#define SYS_

void syscall_init(void);
void syscall_handler(_regs_t* reg);

#endif // SYS_H
