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

static unsigned int g_disk_id  = 0;
static unsigned int g_base_lba = 0; 
static Fat g_fat;
static int g_mounted = 0;

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