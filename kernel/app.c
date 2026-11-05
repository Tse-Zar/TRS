#include <app.h>
#include <fat32.h>
#include <memory_paging.h>

#include <image.h>
#include <stream.h>
#include <keyboard.h>

#define APP_STACK_SIZE (16 * 1024) 

int start_proc(const char *path) {
    File file;
    int bytes;

    int rc = fat_file_open(&file, path, FAT_READ);
    if(rc != FAT_ERR_NONE) 
        return -1;

    unsigned long long img_size  = (file.size + PAGE_SIZE - 1) & ~(unsigned long long)(PAGE_SIZE - 1);
    unsigned long long img_pages = img_size / PAGE_SIZE;
    if(img_pages == 0) img_pages = 1;

    unsigned long long loaded_addr = (unsigned long long)alloc_pages(img_pages);
    if(!loaded_addr) return -3;

    for(unsigned long long i = 0; i < img_pages; i++) {
        make_page_executable(loaded_addr + (i * PAGE_SIZE));
    }

    unsigned long long app_stack = (unsigned long long)alloc_pages(APP_STACK_SIZE / PAGE_SIZE);
    if(!app_stack) {
        free_page((void*)loaded_addr);
        return -4;
    }
    app_stack += APP_STACK_SIZE;

    rc = fat_file_read(&file, (int*)loaded_addr, file.size, &bytes);
    fat_file_close(&file);
    if(rc != FAT_ERR_NONE) 
        return rc;

    unsigned long long saved_rsp;
    __asm__ volatile("mov %%rsp, %0\n\t"
                     "mov %2, %%rsp\n\t"
                     "call *%1\n\t" 
                     "mov %0, %%rsp\n\t"
                     : "=&r"(saved_rsp)
                     : "r"(loaded_addr), "r"(app_stack)
                     : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory", "cc");
    return 0;
}