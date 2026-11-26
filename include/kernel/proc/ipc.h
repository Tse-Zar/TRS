#ifndef IPC_H
#define IPC_H

#include <thread.h>
#include <stdint.h>

#define IPC_MSG_MAX 128        // bytes
#define MAX_IPC_OBJ 7

#define IPC_SEND      (1U << 0)
#define IPC_RECEIVE   (1U << 1)
#define IPC_GRANT     (1U << 2)

typedef uintptr_t word_t;

typedef enum {
    IPC_ENDPOINT = 0,
    IPC_NOTIFICATION,
    IPC_TCB                 // thread control block
} ipc_type_t;

typedef enum {
    IPC_OK = 0,
    IPC_ERR_RIGTHS,
    IPC_ERR_TYPE,
    IPC_ERR_GEN,
    IPC_ERR_LEN,
    IPC_ERR_DEAD
} ipc_err_t;

typedef struct {
    thread_t* head;
    thread_t* tail;
} ipc_queue_t;

typedef struct {
    ipc_type_t type;
    uint       gen;                 // generation for use-after-free
    union {
        struct {
            ipc_queue_t* senders;
            ipc_queue_t* receivers;
        } ep;
        struct {
            word_t word;
            ipc_queue_t waiters;
        } noti;
        struct {
            thread_t* caller;
        } reply;
    } obj;
} ipc_object;

typedef struct {
    ipc_object* obj;
    uint        gen;
    uint        rights;
    word_t      badge;
} ipc_cap_t;

typedef struct {
    word_t msg_regs[IPC_REGS_FOR_MSG];  // fastpath
    uint   len;
    word_t label;
} ipc_msg_t;

ipc_err_t ipc_send(thread_t* sender, const ipc_cap_t* ep, ipc_msg_t* msg);
ipc_err_t ipc_recv(thread_t* receiver, const ipc_cap_t* ep, ipc_msg_t* out, word_t* badge_out);
ipc_err_t ipc_reply_recv(thread_t* caller, const ipc_cap_t* reply, const ipc_cap_t* ep, const ipc_msg_t* next, word_t* badge_out);
ipc_err_t ipc_call(thread_t* caller, const ipc_cap_t* ep, ipc_msg_t* msg);

ipc_err_t ipc_signal(thread_t* th, const ipc_cap_t* noti);
ipc_err_t ipc_wait(thread_t* th, const ipc_cap_t* noti, word_t* out);
ipc_err_t ipc_poll(thread_t* th, const ipc_cap_t* noti, word_t* out);

#endif // IPC_H
