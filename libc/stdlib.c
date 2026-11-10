#include <stdlib.h>
#include <syscall.h>
#include <stdint.h>
static unsigned int next = 623432423;

uint32_t rand(pcg32_random_t* rng) {
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot)  | ( xorshifted << (( - rot ) & 31 ));
}

int process_start(const char *name) {
    return (int)syscall1(SYS_START_PROC, (void*)name);
}