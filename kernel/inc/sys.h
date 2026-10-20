/*
    syscalls
*/

#ifndef SYS_H
#define SYS_H

#include <idt.h>

#define SYS_WRITE 1

void syscall_init(void);
void syscall_handler(_regs_t* reg);

#endif // SYS_H