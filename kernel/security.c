/*
    ========================
    = TSEZAR TSEZAR TSEZAR =
    = -------------------- =
    = Kernel stack protect =
    ========================
*/

#include <security.h>
#include <panic.h>


#define DBG_PORT 0x3F8
static void dbg(char c) {
    __asm__ volatile("outb %0, %1" :: "a"(c), "d"((unsigned short)DBG_PORT));
}

static int cpu_has_rdrand(void) {
    unsigned int a, b, c, d;
    
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
                             : "a"(1U), "c"(0U));

    return (int)((c >> 30) & 1U);
}

unsigned long __stack_chk_guard = 0x9E3779B97F4A7C15UL;

static int rdrand64(unsigned long* out) {
    unsigned char ok = 0;
    int retry = 10;

    do {
        __asm__ volatile("rdrand %0\n\t"
                         "setc %1" : "=r"(*out), "=qm"(ok)
                         :: "cc");

    } while(!ok && --retry);

    return ok;
}

static unsigned long rdtsc64(void) {
    unsigned int low   = 0;
    unsigned int higth = 0;

    __asm__ volatile("rdtsc" : "=a"(low), "=d"(higth));

    return ((unsigned long)higth << 32) | (unsigned long)low;
}

void stack_guard_init(void) {
    unsigned long g = 0;

    if(cpu_has_rdrand() && rdrand64(&g) && g) {
        dbg('H');
        __stack_chk_guard = g;
        return;
    }

    g = rdtsc64() ^ (unsigned long)(unsigned int)(__UINTPTR_TYPE__)&g;
    if(!g)
        g = 0x9E3779B97F4A7C15UL;
    
    dbg('N');
    __stack_chk_guard = g;
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
    kpanic("stack smashing detected", __FILE__, __LINE__);
}