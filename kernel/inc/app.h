#ifndef APP_H
#define APP_H

#include <boootinfo.h>

#define PRO_MAGIC    0x54525350u
#define PRO_HDR_SIZE 64
#define PRO_NAME_MAX 16
#define PRO_MAX_SIZE (1024 * 1024)

#define PRO_FLAG_RING3 0x1

typedef struct {
    unsigned int       magic;
    unsigned int       version;
    unsigned long long entry;
    unsigned long long image_size;
    unsigned long long mem_size;
    unsigned long long flags;
    char               name[PRO_NAME_MAX];
    unsigned long long reserved;
} __attribute__((packed)) pro_hdr_t;

_Static_assert(sizeof(pro_hdr_t) == PRO_HDR_SIZE, ".pro header must be 64 bytes");


#define PRO_OK          0
#define PRO_ERR_NOENT  -1
#define PRO_ERR_BADIMG -2
#define PRO_ERR_NOMEM  -3

void app_init(boot_info_t* bi);

int start_process(const char* path);
#endif // APP_H