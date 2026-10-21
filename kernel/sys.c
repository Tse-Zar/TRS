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

        case SYS_READ: {
            char* buf = (char*)reg->rdx;
            unsigned long long len = (unsigned long long)reg->rdi;

            if (!buf || len == 0) {
                reg->rax = 0;
                break;
            }

            stream_t* kbd = kbd_get_stream();
            unsigned long long i = 0;

            while (i + 1 < len) {
                char ch;
                stream_read(kbd, &ch, 1);
                if (ch == '\n') {
                    break;
                }
                if (ch == '\b') {
                    if (i > 0) {
                        i--;
                        dsp_print("\b");
                    }
                    continue;
                }

                buf[i] = ch;
                i++;

                char echo[2];
                echo[0] = ch;
                echo[1] = '\0';
                dsp_print(echo);
            }

            buf[i] = '\0';
            reg->rax = i;
            break;
        }

        default:
            reg->rax = -1;
            break;
    }
}