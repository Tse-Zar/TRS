#ifndef STDIO_H
#define STDIO_H

#include <types.h>

typedef struct {
    const char* name;
} FILE;

void printf(const char* str);
uint scanf(char* buf, ullong len);

void delay_ms(ullong ms);

/* --- cli command --- */
void cli_clear(void);
void set_color(ullong fg, ullong bg);
void set_crsr(ullong x, ullong y);
void flip(void);

/* --- file command --- */
FILE fopen(const char* name);

#endif //STDIO_H
