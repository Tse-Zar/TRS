#ifndef SYSCALLS_H
#define SYSCALLS_H

#define SYS_WRITE 1
#define SYS_READ  2

unsigned int syscall1(int call, void *one);
unsigned int syscall2(int call, void* one, void* two);

#endif // SYSCALLS_H