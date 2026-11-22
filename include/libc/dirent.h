#ifndef DIRENT_H
#define DIRENT_H

#include <types.h>

#define ATTR_RO      0x01   // readonly
#define ATTR_HIDDEN  0x02
#define ATTR_SYS     0x04
#define ATTR_LABEL   0x08
#define ATTR_DIR     0x10
#define ATTR_ARCHIVE 0x20

#define NAME_MAX 32

typedef struct {
    int fd;
} DIR;

typedef struct {
    char          name[NAME_MAX + 1];
    uint  size;
    uchar attr;
    uchar name_len;
} DirInfo;

int opendir(DIR* dir, const char* name);
int readdir(DIR* dir, DirInfo* info);
int closedir(DIR* dir);

#endif //DIRENT_H
