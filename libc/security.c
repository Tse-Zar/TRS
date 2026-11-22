/*
    =================
    = TSEZAR TSEZAR =
    = ------------- =
    = USER-SPACE -- =
    = protected --- =
    =================
*/

#include <security.h>
#include <syscall.h>

ullong __stack_chk_guard = 0x9E3779B97F4A7C15UL;

void __stack_chk_fail(void) {
    const char msg[] = "\n[prog] stack smashing detected -- terminated\n";

    syscall1(SYS_WRITE, (void*)msg);

    while(1) {
        __asm__ volatile("pause");
    }
}
