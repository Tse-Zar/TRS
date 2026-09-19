/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = Unified kernel panic system ------ =
    ======================================
*/

#ifndef PANIC_H
#define PANIC_H

void kpanic(const char* msg, const char* file, int line);
void kassert_fail(const char* expr, const char* file, int line);

void panic_out_str(const char* s);
void panic_out_hex(unsigned v);
void panic_halt(void);

#define kassert(expr) \
    ((expr) ? (void)0 : kassert_fail(#expr, __FILE__, __LINE__))

#endif // PANIC_H