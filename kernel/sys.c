#include <sys.h>
#include <image.h>

void syscall_init(void) {
    isr_install(SYSCALL_VECTOR, syscall_handler);
}

void syscall_handler(_regs_t* reg) {
    switch (reg->rax) {
        case SYS_WRITE:
            dsp_print((const char*)reg->rdx);
            break;
    }
}