/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = FAT32 glue (ESP) ------------------ =
    ======================================
*/

#ifndef FAT32_H
#define FAT32_H

#include <fat.h>

int fat32_init(unsigned int disk_id);
Fat* fat32_get(void);
int fat32_dir_open(const char* path);
int fat32_dir_read(int slot, DirInfo* info);
int fat32_dir_close(int slot);

#endif // FAT32_H