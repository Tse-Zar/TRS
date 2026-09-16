/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = KEYBOARD DRIVE: read kb_port to -- =
    = --------------------------- iofile =
    ====================================== 
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "stream.h"
#define KB_BUF_SIZE     256
#define KB_DATA_PORT    0x60
#define KB_COM_PORT     0x64

void kb_init(void);
void kb_irq_handler(void);
stream_t* kbd_get_stream(void);

#endif // KEYBOARD_H