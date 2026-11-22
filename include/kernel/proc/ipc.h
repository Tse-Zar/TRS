#ifndef IPC_H
#define IPC_H

#include <thread.h>

typedef struct {
    thread_t* to;
    thread_t* from;
    long long timeouts;
    void* buffer;
} ipc_word;

void __attribute__((noreturn)) ipc_call();
void __attribute__((noreturn)) ipc_recv();
void __attribute__((noreturn)) ipc_();
#endif // IPC_H
