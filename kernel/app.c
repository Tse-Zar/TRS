#include <app.h>
#include <fat32.h>
#include <memory_paging.h>
#include <string.h>
#include <image.h>
#include <stream.h>
#include <keyboard.h>

#define APP_STACK_SIZE (16 * 1024)
#define APP_BSS_SLACK  4

extern void callapp(void* entry, void* slack_top);

int start_proc(const char *path) {
    File file;
    int bytes = 0;

    if(!path || !*path) return 1231;
    
    int rc = fat_file_open(&file, path, FAT_READ);
    if(rc != FAT_ERR_NONE || file.size == 0) 
        return -1;

    unsigned long long img_pages  = (file.size + PAGE_SIZE - 1)/ PAGE_SIZE;
    if(img_pages == 0) img_pages = 1;

    if(img_pages + APP_BSS_SLACK >= MAX_PAGES_FOR_APP) {
        fat_file_close(&file);
        return -1;
    }

    unsigned long long loaded_addr = (unsigned long long)alloc_pages((img_pages + (unsigned long long)APP_BSS_SLACK));
    if(!loaded_addr) {
        fat_file_close(&file);
        return -3;
    }

    unsigned long long app_stack = (unsigned long long)alloc_pages((unsigned long long)APP_STACK_SIZE / (unsigned long long)PAGE_SIZE);
    if(!app_stack) {
        free_page((void*)loaded_addr);
        fat_file_close(&file);
        return -4;
    }

    memset((void*)loaded_addr, 0, (img_pages + APP_BSS_SLACK) * PAGE_SIZE);
    memset((void*)app_stack, 0, APP_STACK_SIZE);
    
    for(unsigned long long i = 0; i < img_pages; i++) {
        make_page_executable(loaded_addr + (i * PAGE_SIZE));
    }

    rc = fat_file_read(&file, (int*)loaded_addr, file.size, &bytes);
    fat_file_close(&file);
    if(rc != FAT_ERR_NONE || (unsigned int)bytes != file.size) {
        for(unsigned long long i = 0; i < img_pages + APP_BSS_SLACK; ++i) {
            free_page((void*)(loaded_addr + (i * PAGE_SIZE)));
        }
        for(unsigned long long i = 0; i < APP_STACK_SIZE / PAGE_SIZE; ++i) {
            free_page((void*)(app_stack + (i * PAGE_SIZE)));
        }

        return 123;
    }

    callapp((void*)loaded_addr, (void*)(app_stack + APP_STACK_SIZE));

    for(unsigned long long i = 0; i < img_pages + APP_BSS_SLACK; ++i) {
            free_page((void*)(loaded_addr + (i * PAGE_SIZE)));
    }
    for(unsigned long long i = 0; i < APP_STACK_SIZE / PAGE_SIZE; ++i) {
        free_page((void*)(app_stack + (i * PAGE_SIZE)));
    }
    
    return 0;
}