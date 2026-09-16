/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = TRS libc - freestanding string ops =
    ======================================
*/

#include "../inc/string.h"
#include <stdint.h>

void* memset(void* dst, int val, size_t len) {
    unsigned char* d = (unsigned char*)dst;
    while(len--) *d++ = (unsigned char)val;

    return dst;
}


// safety slow (rewritte soon)
void* memcpy(void* dst, const void* src, size_t len) {
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;

    while(len > 0 && ((uintptr_t)d & 3) != 0) {
        *d++ = *s++;
        --len;
    }

    int s_is_aligned = ((uintptr_t)s & 3) == 0;

    uint32_t* d32 = (uint32_t*)d;
    uint32_t* s32 = (uint32_t*)s;

    while(len >= 32 && s_is_aligned) {
        d32[0] = s32[0];
        d32[1] = s32[1];
        d32[2] = s32[2];
        d32[3] = s32[3];
        d32[4] = s32[4];
        d32[5] = s32[5];
        d32[6] = s32[6];
        d32[7] = s32[7];

        d32 += 8;
        s32 += 8;
        len -= 32;
    }

    while(len >= 4 && s_is_aligned) {
        *d32++ = *s32++;
        len -= 4;
    }

    d = (unsigned char*)d32;
    s = (unsigned char*)s32;

    while(len > 0) {
        *d++ = *s++;
        len--;
    }
    return dst;
}


void* memmove(void* dst, const void* src, size_t len) {
    if(dst > src) {
        unsigned char* d = (unsigned char*)dst + len;
        unsigned char* s = (unsigned char*)src + len;
        while(len--) *--d = *--s;
        
        return dst;
    } else if(dst < src) {
        return memcpy(dst, src, len);
    } else {
        return dst;
    }
}

// maybe slow
int memcmp(const void* a, const void* b, size_t len) {
    const unsigned char* first = (const unsigned char*)a;
    const unsigned char* second = (const unsigned char*)b;

    while(len--) {
        if(*first != *second) return *first++ - *second++;
    }

    return 0;
}

size_t strlen(const char* s) {
    size_t len = 0;

    while(*s++) {
        len++;
    }
    return len;
}

/* TOO SLOW */
int strcmp(const char* a, const char* b) {
    while(*a != '\0' && *b != '\0') {
        if(*a != *b) break;
        a++;
        b++;
    }

    return *(unsigned const char*)a - *(unsigned const char*)b;
}


int strncmp(const char* a, const char* b, size_t len) {
    while(len-- && (*a != '\0' && *b != '\0')) {
        if(*a != *b) break;
        a++;
        b++;
    }

    return *(unsigned const char*)a - *(unsigned const char*)b;
}