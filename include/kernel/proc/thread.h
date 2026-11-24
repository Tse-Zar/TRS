#ifndef THREAD_H
#define THREAD_H

#include <types.h>
#include <idt.h>

#define USER_BASE    0x0000004000000000ULL
#define USER_TOP     0x0000008000000000ULL
#define KSTACK_PAGES 4
#define MAX_THREADS  64
#define MSG_WORDS    8
#define IPC_MAX_CAPS 32
#define IPC_REGS_FOR_MSG 8

typedef struct {
    uint type;
    uint rights;
    void*        obj;
    uint badge;
    uint gen;
    uint death;
} cap_t;

typedef struct {
    uint label;
    uint words;
    uint caps;
    ullong w[MSG_WORDS];
    uint badge;
} msg_t;

typedef enum {
    TH_FREE = 0,
    TH_READY,
    TH_RUNNING,
    TH_BLOCKED,
    TH_SLEEP,
    TH_DEAD
} th_state_t;

typedef struct {
    ullong cr3;
    uint       pid;

    ullong image_base;
    uint       image_pages;
    ullong ustack_base;
    uint       ustack_pages;

    cap_t              caps[IPC_MAX_CAPS];
} task_t;

typedef struct {
    ullong msg_regs[IPC_REGS_FOR_MSG];
    uint   msg_ingo;
    uint   badge;
    uint  sender_tsd; // sender`s thread-specification-data
} ipc_buffer_t;

typedef struct thread {
    /* --- Context --- */
    _regs_t*           frame;
    th_state_t         state;
    ullong kstack_top;
    ullong ustack_top;
    task_t*            task;
    uint       tid;         // thread id : view-format pid ~ tid for current thread
    int                exit_code;

    /* --- IPC ---*/
    ipc_buffer_t ipc_buf;
} thread_t;


#endif //THREAD_H
