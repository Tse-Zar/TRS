/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = TRS libc - freestanding string ops =
    ======================================
*/

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void* memset(void* dst, int val, size_t len);
void* memcpy(void* dst, const void* src, size_t len);
void* memmove(void* dst, const void* src, size_t len);
int memcmp(const void* a, const void* b, size_t len);

size_t strlen(const char* s);
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t len);

#endif // STRING_H