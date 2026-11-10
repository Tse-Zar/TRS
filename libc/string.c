/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = TRS libc - freestanding string ops =
    ======================================
*/

#include <string.h>
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

    uint64_t* d64 = (uint64_t*)d;
    uint64_t* s64 = (uint64_t*)s;

    while(len >= 64 && s_is_aligned) {
        d64[0] = s64[0];
        d64[1] = s64[1];
        d64[2] = s64[2];
        d64[3] = s64[3];
        d64[4] = s64[4];
        d64[5] = s64[5];
        d64[6] = s64[6];
        d64[7] = s64[7];

        d64 += 8;
        s64 += 8;
        len -= 64;
    }

    while(len >= 8 && s_is_aligned) {
        *d64++ = *s64++;
        len -= 8;
    }

    d = (unsigned char*)d64;
    s = (unsigned char*)s64;

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
        if(*first != *second) return *first - *second;
        first++;
        second++;
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
    if(len == 0) return 0;
    
    while(len-- && (*a != '\0' && *b != '\0')) {
        if(*a != *b) break;
        a++;
        b++;
    }

    return *(unsigned const char*)a - *(unsigned const char*)b;
}

char* strcat(char* dst, const char* src) {
    char* ptr = dst;

    while(*dst != '\0') dst++;

    while((*dst++ = *src++)) {}

    return ptr;
}

char* strrchr(const char *str, int ch) {
    char* last = NULL;

    do {
        if(*str == (unsigned char)ch) {
            last = (char*)str;
        }
    } while(*str++);

    return last;
}

char* strcpy(char *dst, const char *src) {
    if(!dst || !src) return NULL;
    
    char* ptr = dst;

    while((*dst++ = *src++)) {}
    return ptr;
}