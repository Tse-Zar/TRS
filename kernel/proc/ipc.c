#include <ipc.h>
#include <thread.h>
#include <types.h>

static ipc_object objects[MAX_IPC_OBJ];

ipc_err_t ipc_send(thread_t* sender, const ipc_cap_t* ep, ipc_msg_t* msg) {

    return IPC_OK;
}
