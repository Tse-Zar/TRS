#include <app.h>
#include <fat32.h>
#include <memory_paging.h>
#include <string.h>
#include <stream.h>
#include <thread.h>
#include <types.h>

#define APP_STACK_SIZE (64 * 1024)
#define APP_BSS_SLACK  4

static void app_exit_tramp(void) {
    //thread_exit(0);
    while(1) {
        __asm__ volatile("sti; hlt");
    }
}

static ullong read_cr3(void) {
    ullong v;
    __asm__ volatile("mov %%cr3, %0" : "=r"(v));
    return v;
}

int start_proc(const char *path) {
    File file;
    int bytes = 0;

    if(!path || !*path || strlen(path) > PRO_NAME_MAX)
        return PRO_ERR_BADIMG;

    int rc = fat_file_open(&file, path, FAT_READ);
    if(rc != FAT_ERR_NONE || file.size == 0)
        return PRO_ERR_NOENT;

    ullong img_pages  = (file.size + PAGE_SIZE - 1)/ PAGE_SIZE;
    if(img_pages == 0)
        img_pages = 1;

    ullong total = img_pages + APP_BSS_SLACK;
    if(total >= MAX_PAGES_FOR_APP) {
        fat_file_close(&file);
        return PRO_ERR_TOLARGE;
    }

    ullong loaded_addr = (ullong)alloc_pages(total);
    if(!loaded_addr) {
        fat_file_close(&file);
        return PRO_ERR_NOMEM;
    }

    ullong stack = (ullong)alloc_pages((ullong)APP_STACK_SIZE / (ullong)PAGE_SIZE);
    if(!stack) {
        for(ullong i = 0; i < total; ++i) {
            free_page((void*)(loaded_addr + i * PAGE_SIZE));
        }
        fat_file_close(&file);
        return PRO_ERR_NOMEM;
    }

    memset((void*)loaded_addr, 0, (img_pages + APP_BSS_SLACK) * PAGE_SIZE);
    memset((void*)stack, 0, APP_STACK_SIZE);

    for(ullong i = 0; i < img_pages; i++) {
        make_page_executable(loaded_addr + (i * PAGE_SIZE));
    }

    rc = fat_file_read(&file, (int*)loaded_addr, file.size, &bytes);
    fat_file_close(&file);

    if(rc != FAT_ERR_NONE || (uint)bytes != file.size) {
        for(ullong i = 0; i < img_pages + APP_BSS_SLACK; ++i) {
            free_page((void*)(loaded_addr + (i * PAGE_SIZE)));
        }
        for(ullong i = 0; i < APP_STACK_SIZE / PAGE_SIZE; ++i) {
            free_page((void*)(stack + (i * PAGE_SIZE)));
        }

        return PRO_ERR_BADIMG;
    }

    ullong stack_top = stack + APP_STACK_SIZE;
    *(ullong*)(stack_top - 8) = (ullong)app_exit_tramp;
/*
    thread_t* t = thread_create(loaded_addr, read_cr3() & PTE_ADDR, stack_top - 8, 0);
    if(!t) {
            for(ullong i = 0; i < img_pages + APP_BSS_SLACK; ++i) {
            free_page((void*)(loaded_addr + (i * PAGE_SIZE)));
        }
        for(ullong i = 0; i < APP_STACK_SIZE / PAGE_SIZE; ++i) {
            free_page((void*)(stack + (i * PAGE_SIZE)));
        }
        return PRO_ERR_NOMEM;
    }
    t->task->image_base   = loaded_addr;
    t->task->ustack_base  = stack;
    t->task->ustack_pages = APP_STACK_SIZE / PAGE_SIZE;
    t->task->image_pages  = total;
*/
    return PRO_OK;
}
