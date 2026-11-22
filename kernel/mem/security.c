/*
    ========================
    = TSEZAR TSEZAR TSEZAR =
    = -------------------- =
    = Kernel stack protect =
    ========================
*/

#include <security.h>
#include <panic.h>
#include <types.h>


static int cpu_has_rdrand(void) {
    uint a, b, c, d;

    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
                             : "a"(1U), "c"(0U));

    return (int)((c >> 30) & 1U);
}

ulong __stack_chk_guard = 0x9E3779B97F4A7C15UL;

static int rdrand64(ulong* out) {
    uchar ok = 0;
    int retry = 10;

    do {
        __asm__ volatile("rdrand %0\n\t"
                         "setc %1" : "=r"(*out), "=qm"(ok)
                         :: "cc");

    } while(!ok && --retry);

    return ok;
}

static ulong rdtsc64(void) {
    uint low   = 0;
    uint hight = 0;

    __asm__ volatile("rdtsc" : "=a"(low), "=d"(hight));

    return ((ulong)hight << 32) | (ulong)low;
}

void stack_guard_init(void) {
    ulong g = 0;

    if(cpu_has_rdrand() && rdrand64(&g) && g) {
        __stack_chk_guard = g;
        return;
    }

    g = rdtsc64() ^ (ulong)(uint)(__UINTPTR_TYPE__)&g;
    if(!g)
        g = 0x9E3779B97F4A7C15UL;

    __stack_chk_guard = g;
}

__attribute__((noreturn)) void __stack_chk_fail(void) {
    kpanic("stack smashing detected", __FILE__, __LINE__);
}
