#ifndef MEMORY_PAGING_H
#define MEMORY_PAGING_H

#define PAGE_SIZE 4096

typedef struct {
    unsigned long address;
    unsigned char flag;
    unsigned long shift;
} mmpage;

void paging_init(void);
#endif // MEMORY_PAGING_H