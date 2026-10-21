#include <app.h>
#include <fat32.h>
#define PAGE_SIZE 4096
#define APP_STACK_SIZE (16 * 1024) 

int start_proc(const char *path) {
    File file;
    int bytes;

    int rc = fat_file_open(&file, path, FAT_READ);
    if(rc != FAT_ERR_NONE) return -1;
    rc = fat_file_read(&file, (int*)PRO_BASE, file.size, &bytes);
    if(rc != FAT_ERR_NONE) return -2;


    __asm__ volatile("call %0" :
                     : "r"((void*)PRO_BASE)
                     : "memory", "cc");
    return 0;
}