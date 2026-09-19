#ifndef MEMORY_PAGGING_H
#define MEMORY_PAGGING_H

#define PAGE_SIZE 4096

typedef struct {
    unsigned long addres;
    unsigned char flag;
    unsigned long shift;
} mmpage;

void pagging_init(void);
#endif // MEMORY_PAGGING_H