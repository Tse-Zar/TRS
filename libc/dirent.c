#include <dirent.h>
#include <syscall.h>
#include <stdbool.h>

int opendir(DIR *dir, const char *name) {
    if(!dir || !name) return -1;

    int fd = (int)syscall1(SYS_OPENDIR, (void*)name);
    if(fd < 0) return fd;

    dir->fd = fd;

    return 0;
}

int readdir(DIR *dir, DirInfo *info) {
    if(!dir || !info) return -1;
    DirInfo inf;

    while(true) {
        int rc = (int)syscall2(SYS_READDIR, (void*)(long)dir->fd, &inf);
        if(rc != 0) return rc;
        if(inf.attr & ATTR_LABEL) continue;

        if(inf.name[0] == '.' && (inf.name[1] == '\0' || (inf.name[1] == '.' && inf.name[2] == '\0')))
            continue;
        *info = inf;
        return 0;
    }
} 

int closedir(DIR *dir) {
    if(!dir) return -1;

    int rc = (int)syscall1(SYS_CLOSEDIR, (void*)(long)dir->fd);
    dir->fd = -1;
    return rc;
}