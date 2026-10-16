/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = ATA PIO block device driver ------ =
    = polled mode (nIEN), LBA28 -------- =
    = ====================================
*/

#include <disk.h>
#include <io.h>
#include <string.h>

#define ATA_REG_DATA     0
#define ATA_REG_FEATURES 1
#define ATA_REG_SECCOUNT 2
#define ATA_REG_LBA0     3
#define ATA_REG_LBA1     4
#define ATA_REG_LBA2     5
#define ATA_REG_DRIVE    6
#define ATA_REG_STATUS   7
#define ATA_REG_COMMAND  7

#define ATA_SR_BSY 0x80
#define ATA_SR_DF  0x20
#define ATA_SR_DRQ 0x08
#define ATA_SR_ERR 0x01

#define ATA_CMD_READ_PIO 0x20
#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_WRITE_PIO 0x30
#define ATA_CMD_FLUSH     0xE7


#define ATA_LBA28_MAX 0x0FFFFFFFFULL

static disk_t disks[DISK_MAX];
static unsigned int disk_num = 0;

static void ata_delay400(unsigned short ctrl) {
    inb(ctrl); inb(ctrl); inb(ctrl); inb(ctrl);
}

static int ata_wait_bsy(unsigned short io) {
    for(unsigned long i = 0; i < 10000000; ++i) {
        if(!(inb(io + ATA_REG_STATUS) & ATA_SR_BSY)) return 0;
    }

    return -1;
}

static int ata_wait_drq(unsigned short io) {
    for(unsigned long i = 0; i < 10000000; ++i) {
        unsigned char st = inb(io + ATA_REG_STATUS);
        if(st & (ATA_SR_ERR | ATA_SR_DF)) return -1;
        if(st & ATA_SR_DRQ) return 0;
    }

    return -1;
}

static int ata_identify(unsigned short io, unsigned short ctrl, unsigned char drive, disk_t* out) {
    outb(ctrl, 0x02);

    outb(io + ATA_REG_DRIVE, (unsigned char)(0xA0 | (drive << 4)));
    ata_delay400(ctrl);

    outb(io + ATA_REG_SECCOUNT, 0);
    outb(io + ATA_REG_LBA0, 0);
    outb(io + ATA_REG_LBA1, 0);
    outb(io + ATA_REG_LBA2, 0);

    outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);
    ata_delay400(ctrl);

    unsigned char st = inb(io + ATA_REG_STATUS);
    if(st == 0 || st == 0xFF) return -1;

    if(ata_wait_bsy(io) != 0) return -1;
    if(inb(io + ATA_REG_LBA1) || inb(io + ATA_REG_LBA2)) return -1;
    if(ata_wait_drq(io) != 0) return -1;

    unsigned short id[256];
    for(int i = 0; i < 256; ++i) id[i] = inw(io + ATA_REG_DATA);

    unsigned long long sectors = ((unsigned long long)id[61] << 16) | (unsigned long long)id[60];
    if(sectors == 0) return -1;
    if(sectors > ATA_LBA28_MAX) sectors = ATA_LBA28_MAX;

    out->sectors = sectors;
    out->present = 1;
    return 0;
} 

int disk_init(void) {
    static const unsigned short io_bases[2]   = { 0x1F0, 0x170 };
    static const unsigned short ctrl_bases[2] = {0x3F6, 0x376 };
    
    disk_num = 0;

    for(unsigned int ch/*anell*/ = 0; ch < 2; ++ch) {
        for(unsigned int drive = 0; drive < 2; ++drive) {
            if(disk_num >= DISK_MAX) return (int)disk_num;

            disk_t disk;
            memset(&disk, 0, sizeof(disk));

            if(ata_identify(io_bases[ch], ctrl_bases[ch], (unsigned char)drive, &disk) == 0) {
                disk.io_base   = io_bases[ch];
                disk.ctrl_base = ctrl_bases[ch];
                disk.drive     = (unsigned char)drive;
                disks[disk_num++] = disk;
            }
        }
    }

    return (int)disk_num;
}

unsigned disk_count(void) { return disk_num; }

const disk_t* disk_get(unsigned int id) {
    return (id < disk_num) ? &disks[id] : 0;
}

int read_disk(unsigned int id, unsigned long long lba, unsigned int count, void* buf) {
    if(id >= disk_num || !buf) return -1;
    if(count == 0 || count > 255) return -1;
    if(lba + count > disks[id].sectors) return -1;
    
    unsigned short io    = disks[id].io_base;
    unsigned short ctrl  = disks[id].ctrl_base;
    unsigned char  drive = disks[id].drive;

    if(ata_wait_bsy(io) != 0) return -1;

    outb(io + ATA_REG_DRIVE, (unsigned char)(0xE0 | (drive << 4) | ((lba >> 24) &0x0F)));
    ata_delay400(ctrl);

    outb(io + ATA_REG_FEATURES, 0);
    outb(io + ATA_REG_SECCOUNT, (unsigned char)count);
    outb(io + ATA_REG_LBA0, (unsigned char)(lba & 0xFF));
    outb(io + ATA_REG_LBA1, (unsigned char)((lba >> 8) & 0xFF));
    outb(io + ATA_REG_LBA2, (unsigned char)((lba >> 16) & 0xFF));
    outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

    unsigned short *p = (unsigned short*)buf;
    for(unsigned int s = 0; s < count; ++s) {
        if(ata_wait_bsy(io) != 0) return -1;
        if(ata_wait_drq(io) != 0) return -1;

        for(int i = 0; i < 256; ++i) *p++ = inw(io + ATA_REG_DATA);
        ata_delay400(ctrl);
    }
    return 0;
}


int write_disk(unsigned int id, unsigned long long lba, unsigned int count, const void* buf) {
    if(id >= disk_num || !buf) return -1;
    if(count == 0 || count > 255) return -1;
    if(lba + count > disks[id].sectors) return -1;

    unsigned short io    = disks[id].io_base;
    unsigned short ctrl  = disks[id].ctrl_base;
    unsigned char  drive = disks[id].drive;

    if(ata_wait_bsy(io) != 0) return -1;

    outb(io + ATA_REG_DRIVE, (unsigned char)(0xE0 | (drive << 4) | ((lba >> 24) & 0x0F)));
    ata_delay400(ctrl);

    outb(io + ATA_REG_FEATURES, 0);
    outb(io + ATA_REG_SECCOUNT, (unsigned char)count);
    outb(io + ATA_REG_LBA0, (unsigned char)(lba & 0xFF));
    outb(io + ATA_REG_LBA1, (unsigned char)((lba >> 8) & 0xFF));
    outb(io + ATA_REG_LBA2, (unsigned char)((lba >> 16) & 0xFF));
    outb(io + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);

    const unsigned short* p = (const unsigned short*)buf;
    for(unsigned int s = 0; s < count; ++s) {
        if(ata_wait_bsy(io) != 0) return -1;
        if(ata_wait_drq(io) != 0) return -1;

        for(int i = 0; i < 256; ++i) outw(*p++, io + ATA_REG_DATA);
        ata_delay400(ctrl);
    }
    
    if(ata_wait_bsy(io) != 0) return -1;
    outb(io + ATA_REG_COMMAND, ATA_CMD_FLUSH);
    if(ata_wait_bsy(io) != 0) return -1;

    return 0;
}