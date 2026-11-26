#include <thread.h>
#include <gdt.h>
#include <memory_paging.h>
#include <string.h>
#include <panic.h>
#include <timer.h>
#include <types.h>

static thread_t  threads[MAX_THREADS];
static thread_t* current_thread = NULL;
static uint      thread_count   = 0;

static thread_t run_queue_head = {0};
static thread_t run_queue_tail = {0};
static uint     run_queue_len  = 0;

void sched_init(void) {
    memset(threads, 0, sizeof(threads));
    run_queue_head.next = &run_queue_tail;
    run_queue_head.prev = NULL;
    run_queue_tail.next = NULL;
    run_queue_tail.prev = &run_queue_head;
}
