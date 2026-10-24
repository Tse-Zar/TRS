/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = Unified kernel panic system ------ =
    ======================================
*/

#ifndef PANIC_H
#define PANIC_H

#include <idt.h>

void kpanic(const char* msg, const char* file, int line) __attribute__((noreturn));
void kassert_fail(const char* expr, const char* file, int line) __attribute__((noreturn));
void kpanic_regs(const _regs_t* reg, const char* msg) __attribute__((noreturn));

void panic_halt(void) __attribute__((noreturn));

#define KASSERT(expr) do { if(!(expr)) kassert_fail(#expr, __FILE__, __LINE__); } while(0)

#endif // PANIC_H