#ifndef APP_H
#define APP_H

#define PRO_MAGIC    0x54525350u
#define PRO_HDR_SIZE 64
#define PRO_NAME_MAX 16
#define PRO_MAX_SIZE (1024 * 1024)

#define PRO_FLAG_RING3 0x1
#define PRO_BASE 0x400000ULL

#define PRO_OK          0
#define PRO_ERR_NOENT  -1
#define PRO_ERR_BADIMG -2
#define PRO_ERR_NOMEM  -3

int start_proc(const char* path);
#endif // APP_H