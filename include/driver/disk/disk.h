/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = Blocking device layer ATA PIO ---- =
    ======================================
*/

#ifndef DISK_H
#define DISK_H

#include <types.h>

#define DISK_SECTOR_SIZE 512
#define DISK_MAX         4

typedef struct {
    ullong sectors;
    ushort     io_base;
    ushort     ctrl_base;
    uchar      drive;
    uchar      present;
} disk_t;

int disk_init(void);

uint disk_count(void);
const disk_t* disk_get(uint id);

int read_disk(uint id, ullong lba, uint count, void* buf);
int write_disk(uint id, ullong lba, uint count, const void* buf);
#endif // DISK_H
