/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = FAT32 glue: GPT ESP -> fat_mount -- =
    = ====================================
*/

#include <fat32.h>
#include <disk.h>
#include <gpt.h>
#include <string.h>

#define MAX_OPENDIRS 10

typedef struct {
    Dir dir;
    int used;
    int need_next;
} DirSlot;

static unsigned int g_disk_id  = 0;
static unsigned int g_base_lba = 0; 
static Fat g_fat;
static int g_mounted = 0;

static DirSlot g_dirs[MAX_OPENDIRS];

static bool fat32_read_sect(uint8_t* buf, uint32_t sect) {
    return read_disk(g_disk_id, (unsigned long long)g_base_lba + sect, 1, buf) == 0;
}

static bool fat32_write_sect(const uint8_t* buf, uint32_t sect) {
    return write_disk(g_disk_id, (unsigned long long)g_base_lba + sect, 1, buf) == 0;
}

int fat32_init(unsigned int disk_id) {
    gpt_part_t esp;
    if(gpt_find_esp(disk_id, &esp) != 0) return FAT_ERR_NOFAT;

    g_disk_id  = disk_id;
    g_base_lba = (unsigned int)esp.frist_lba;

    DiskOps ops;
    ops.read  = fat32_read_sect;
    ops.write = fat32_write_sect;

    int err = fat_mount(&ops, 0, &g_fat, "dsk");
    if(err != FAT_ERR_NONE) return err;

    g_mounted = 1;
    return FAT_ERR_NONE;
}

Fat* fat32_get(void) {
    return g_mounted ? &g_fat : 0;
}

int fat32_dir_open(const char* path) {
    if(!g_mounted || !path) return FAT_ERR_PARAM;
    for(int i = 0; i < MAX_OPENDIRS; ++i) {
        if(g_dirs[i].used) continue;
        int rc = fat_dir_open(&g_dirs[i].dir, path);
        if(rc != FAT_ERR_NONE) return rc;
        g_dirs[i].used = 1;
        g_dirs[i].need_next = 0;
        return i;
    }

    return FAT_ERR_FULL;
}

int fat32_dir_read(int slot, DirInfo* info) {
    if(slot < 0 || slot >= MAX_OPENDIRS || !info || !g_dirs[slot].used) return FAT_ERR_PARAM;

    if(g_dirs[slot].need_next) {
        int rc = fat_dir_next(&g_dirs[slot].dir);
        if(rc != FAT_ERR_NONE) return rc;
    }

    g_dirs[slot].need_next = 1;
    return fat_dir_read(&g_dirs[slot].dir, info);
}

int fat32_dir_close(int slot) {
    if(slot < 0 || slot >= MAX_OPENDIRS || !g_dirs[slot].used) return FAT_ERR_PARAM;
    memset(&g_dirs[slot], 0, sizeof(DirSlot));
    return FAT_ERR_NONE;
}