#ifndef THREAD_H
#define THREAD_H

#include <types.h>
#include <idt.h>

#define KSTACK_PAGES 4
#define MAX_THREADS  128
#define MSG_WORDS    8
#define IPC_MAX_CAPS 16
#define IPC_REGS_FOR_MSG 8

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
    uint   pid;

    ullong image_base;
    uint   image_pages;
    ullong ustack_base;
    uint   ustack_pages;
} task_t;

typedef struct thread {
    /* --- Context --- */
    _regs_t*   frame;
    th_state_t state;
    ullong     kstack_top;
    task_t*    task;        // thread can have some tasks
    uint       tid;         // thread id
    int        exit_code;   // thread exit code

    /* --- IPC --- */
    struct {
        uint   label;
        uint   sender_tid;
        ullong regs[MSG_WORDS];
        uint   valid;
    } ipc_recv;

    /* --- wait list --- */
    struct thread* next;
    struct thread* prev;
} thread_t;

/* --- scheduler --- */
void           sched_init(void);
thread_t*      sched_next(void);
void           sched_switch(_regs_t* frame);
void           sched_block(thread_t* th);
void           sched_wake(thread_t* th);
void           sched_on_timer(_regs_t* reg);
void           sched_on_syscall(_regs_t *reg);
void           thread_exit(int code);
thread_t*      thread_create(ullong addr, ullong cr3, ullong stack_top, int user);

#endif //THREAD_H
