#ifndef TIMER_H
#define TIMER_H

#include <idt.h>

#define TIMER_HZ 100U

void timer_init(uint hz);
void timer_handler(_regs_t* reg);

ullong timer_ticks(void);
ullong timer_uptime_ms(void);
ullong timer_hz(void);

#endif //TIMER_H
