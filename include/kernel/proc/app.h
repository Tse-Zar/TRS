#ifndef APP_H
#define APP_H

#include <types.h>

#define PRO_MAGIC    0x54525350u // soon
#define PRO_NAME_MAX 16

/* --- PROgrams error codes --- */
#define PRO_OK           0
#define PRO_ERR_NOENT   -1
#define PRO_ERR_BADIMG  -2
#define PRO_ERR_NOMEM   -3
#define PRO_ERR_TOLARGE -4

typedef struct {
    uint magic;
    uint mem_pages;
    uint entry_off;
    char         name[PRO_NAME_MAX];
} __attribute__((packed))pro_hdr_t; // soon

#define PRO_HDR_SIZE sizeof(pro_hdr_t)

int start_proc(const char* path);
#endif // APP_H
