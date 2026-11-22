#include <timer.h>
#include <pic.h>
#include <io.h>
#include <thread.h>

#define PIT_FREQ 1193182U
#define PIT_CMD  0x43
#define PIT_CH0  0x40

static volatile ullong ticks;
static uint hz_actual = TIMER_HZ;

void timer_init(uint hz) {
    if(hz < 19)
        hz = 19;
    if(hz > PIT_FREQ)
        hz = PIT_FREQ;

    uint div = PIT_FREQ / hz;
    if(!div)
        div = 1;
    if(div > 65535)
        div = 65535;
    hz_actual = PIT_FREQ / div;

    outb(PIT_CMD, 0x36);
    outb(PIT_CH0, (uchar)(div & 0xFF));
    outb(PIT_CH0, (uchar)((div >> 8) & 0xFF));

    irq_install(0, timer_handler);
    pic_unmask(0);
}

void timer_handler(_regs_t *reg) {
    (void)reg;
    ticks++;
}

ullong timer_ticks(void) {
    return ticks;
}

ullong timer_hz(void) {
    return hz_actual;
}

ullong timer_uptime_ms(void) {
    return  ticks * 1000ULL / hz_actual;
}
