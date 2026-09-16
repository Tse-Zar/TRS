#include <mem.h>

void* malloc(unsigned long size) {
    (void)size;
    return (void*)0;
}

void* calloc(unsigned long num, unsigned long size) {
    (void)num;
    (void)size;
    return (void*)0;
}

void* realloc(void* ptr, unsigned long new_size) {
    (void)ptr;
    (void)new_size;
    return (void*)0;
}

void free(void* ptr) {
    (void)ptr;
}