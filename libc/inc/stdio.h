#ifndef STDIO_H
#define STDIO_H

typedef struct {
    const char* name;
} FILE;

void printf(const char* str);
unsigned int scanf(char* buf, unsigned long long len);

/* --- cli command --- */
void cli_clear(void);
void set_color(unsigned long long fg, unsigned long long bg);
void set_crsr(unsigned long long x, unsigned long long y);

/* --- file command --- */
FILE fopen(const char* name);

#endif //STDIO_H