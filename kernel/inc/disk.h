/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = Blocking device layer ATA PIO ---- =
    ======================================
*/

#ifndef DISK_H
#define DISK_H

#define DISK_SECTOR_SIZE 512
#define DISK_MAX         4

typedef struct {
    unsigned long long sectors;
    unsigned short     io_base;
    unsigned short     ctrl_base;
    unsigned char      drive;
    unsigned char      present;     
} disk_t;

int disk_init(void);

unsigned int disk_count(void);
const disk_t* disk_get(unsigned int id);

int read_disk(unsigned int id, unsigned long long lba, unsigned int count, void* buf);
int write_disk(unsigned int id, unsigned long long lba, unsigned int count, const void* buf);
#endif // DISK_H 