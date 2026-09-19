/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = TRS LIBC Dynamic Memory header --- =
    = ====================================
*/

#ifndef MEM_H
#define MEM_H

#define HEAP_SIZE 1024 * 64

void* malloc(unsigned long size);
void* calloc(unsigned long num_of_elements, unsigned long size_of_elements);
void* realloc(void* ptr, unsigned long new_size);

void free(void* ptr);

#endif //MEM_H