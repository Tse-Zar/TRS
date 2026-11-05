#ifndef APP_H
#define APP_H

#define PRO_MAGIC    0x54525350u
#define PRO_NAME_MAX 16
#define PRO_FLAG_RING3 0x1

#define PRO_OK           0
#define PRO_ERR_NOENT   -1
#define PRO_ERR_BADIMG  -2
#define PRO_ERR_NOMEM   -3
#define PRO_ERR_TOLARGE -4

typedef struct {
    unsigned int magic;
    unsigned int mem_pages;
    unsigned int entry_off;
    char         name[PRO_NAME_MAX];
} __attribute__((packed))pro_hdt_t;

#define PRO_HDR_SIZE sizeof(pro_hdr_t)

int start_proc(const char* path);
#endif // APP_H