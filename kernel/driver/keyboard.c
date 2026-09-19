/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    ======================================
    = KEYBOARD DRIVE: read kb_port to -- =
    = --------------------------- iofile =
    ====================================== 
*/

#include <stream.h>
#include <keyboard.h>
#include <io.h>
#include <idt.h>
#include <pic.h>
#include <stdbool.h>

static const char scancode_map[58][2] = {
    [0x02] = {'1','!'}, [0x03] = {'2','@'}, [0x04] = {'3','#'},
    [0x05] = {'4','$'}, [0x06] = {'5','%'}, [0x07] = {'6','^'},
    [0x08] = {'7','&'}, [0x09] = {'8','*'}, [0x0A] = {'9','('},
    [0x0B] = {'0',')'}, [0x0C] = {'-','_'}, [0x0D] = {'=','+'},
    [0x0F] = {'\t','\t'}, [0x0E] = {'\b', '\b'},
    [0x10] = {'q','Q'}, [0x11] = {'w','W'}, [0x12] = {'e','E'},
    [0x13] = {'r','R'}, [0x14] = {'t','T'}, [0x15] = {'y','Y'},
    [0x16] = {'u','U'}, [0x17] = {'i','I'}, [0x18] = {'o','O'},
    [0x19] = {'p','P'}, [0x1A] = {'[','{'}, [0x1B] = {']','}'},
    [0x1C] = {'\n','\n'}, 
    [0x1E] = {'a','A'}, [0x1F] = {'s','S'}, [0x20] = {'d','D'},
    [0x21] = {'f','F'}, [0x22] = {'g','G'}, [0x23] = {'h','H'},
    [0x24] = {'j','J'}, [0x25] = {'k','K'}, [0x26] = {'l','L'},
    [0x27] = {';',':'}, [0x28] = {'\'','\"'}, [0x29] = {'`','~'},
    [0x2B] = {'\\','|'},
    [0x2C] = {'z','Z'}, [0x2D] = {'x','X'}, [0x2E] = {'c','C'},
    [0x2F] = {'v','V'}, [0x30] = {'b','B'}, [0x31] = {'n','N'},
    [0x32] = {'m','M'}, [0x33] = {',','<'}, [0x34] = {'.','>'},
    [0x35] = {'/','?'}, [0x39] = {' ',' '},
};

static stream_t kb_stream;
static unsigned char ext_prefix = 0;

static bool shift_down = false;
static bool caps_down  = false;

stream_t* kbd_get_stream(void) { return &kb_stream; }

void kb_init(void) {
    stream_init(&kb_stream, ST_READ);
    irq_install(IRQ_KBD_LINE, kb_irq_handler);
    
    while(inb(KB_COM_PORT) & 0x01) inb(KB_DATA_PORT);

    pic_unmask(1);
}

static void kb_done(void) {
    outb(PIC1_CMD, PIC_EOI);
}

void kb_irq_handler(void) {
    unsigned char sc = inb(KB_DATA_PORT);

    if(sc == 0x0E) {
        ringbuf_put(&kb_stream.rb, '\b');
        kb_done();
        return;
    }
    if(sc == 0xE0) {
        ext_prefix = 1;
        kb_done(); 
        return;
    }
    bool release = (sc & 0x80) != 0;

    unsigned char code = sc & 0x7F;

    if(code == 0x2A || code == 0x36) {
        shift_down = !release;
        kb_done();
        return;
    }
    if(code == 0x3A && !release) {
        caps_down = !caps_down;
        kb_done();
        return;
    }

    if(!release && code < 58 && scancode_map[code][0]) {
        char c = scancode_map[code][shift_down ? 1 : 0];
        if(caps_down && c >= 'a' && c <= 'z' && !shift_down) c -= 32;

        if(ringbuf_put(&kb_stream.rb, (unsigned char)c) != 0) {
            kb_stream.overflow++;
        }
    }
    
    ext_prefix = 0;
    kb_done();
    return;
}