/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = TRS LIBC Dynamic Memory  --------- =
    = ====================================
*/

#include <mem.h>

void* malloc(ulong size) {
    (void)size;
    return (void*)0;
}

void* calloc(ulong num, ulong size) {
    (void)num;
    (void)size;
    return (void*)0;
}

void* realloc(void* ptr, ulong new_size) {
    (void)ptr;
    (void)new_size;
    return (void*)0;
}

void free(void* ptr) {
    (void)ptr;
}
