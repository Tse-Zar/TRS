#include <sys.h>
#include <image.h>
#include <keyboard.h>
#include <stream.h>

void syscall_init(void) {
    isr_install(SYSCALL_VECTOR, syscall_handler);
}

void syscall_handler(_regs_t* reg) {
    switch (reg->rax) {
        case SYS_WRITE:
            if(!reg->rdx) return;

            dsp_print((const char*)reg->rdx);
            break;
        case SYS_READ:
            ;
            stream_t* k = kbd_get_stream();
    
            break;
    }
}