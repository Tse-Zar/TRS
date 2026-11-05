#ifndef STDLIB_H
#define STDLIB_H

#include <std.h>
#include <mem.h>
#include <string.h>
#include <stdint.h>

/* --- pseudo-rand --- */
#include "../pcg/pcg.h"


/* --- process --- */
void process_start(const char* name);

#endif //STDLIB_H