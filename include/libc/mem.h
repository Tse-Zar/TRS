/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = TRS LIBC Dynamic Memory header --- =
    = ====================================
*/

#ifndef MEM_H
#define MEM_H

#include <types.h>

#define HEAP_SIZE (1024 * 64)

void* malloc(ulong size);
void* calloc(ulong num_of_elements, ulong size_of_elements);
void* realloc(void* ptr, ulong new_size);

void free(void* ptr);

#endif //MEM_H
