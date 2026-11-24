#include <sys.h>
#include <image.h>
#include <keyboard.h>
#include <stream.h>
#include <fat32.h>
#include <string.h>
#include <app.h>
#include <thread.h>
#include <timer.h>
#include <panic.h>
#include <gdt.h>
#include <types.h>

#define MSR_EFER        0xC0000080
#define MSR_STAR        0xC0000081
#define MSR_LSTAR       0xC0000082
#define MSR_FMASK       0xC0000084
#define EFER_SCE        (1ULL << 0)
#define SYSCALL_FMASK   0x44700ULL
#define ABI_NAME_MAX    32

ullong kernel_rsp;
ullong user_rsp;

extern void syscall_entry(void);

static void wrmsr(uint msr, ullong v) {
    __asm__ volatile("wrmsr" :: "c"(msr), "a"((uint)v), "d"((uint)(v >> 32)));
}

typedef struct {
    char          name[ABI_NAME_MAX + 1];
    uint  size;
    uchar attr;
    uchar name_len;
} AbiDirInfo;

void syscall_init(void) {
    uint a, b, c, d;
    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0x80000001U));

    if(!(d & (1U << 11)))
        kpanic("CPU has no SYSCALL", __FILE__, __LINE__);

    uint low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(MSR_EFER));
    wrmsr(MSR_EFER, (((ullong)high << 32) | low) | EFER_SCE);

    ullong star = ((ullong)0x10 << 48) | ((ullong)GDT_KERNEL_CODE << 32);

    wrmsr(MSR_STAR, star);
    wrmsr(MSR_LSTAR, (ullong)syscall_entry);
    wrmsr(MSR_FMASK, SYSCALL_FMASK);

    isr_install(SYSCALL_VECTOR, syscall_handler);
}

_regs_t* syscall_dispatch(_regs_t* reg) {
    syscall_handler(reg);
    //return sched_on_syscall(reg);
    return reg;
}

void syscall_handler(_regs_t* reg) {
    switch (reg->rax) {
        case SYS_WRITE:
            if(!reg->rdx) return;
            dsp_print((const char*)reg->rdx);
            break;

        case SYS_READ: {
            char* buf = (char*)reg->rdx;
            ullong len = (ullong)reg->rdi;

            if (!buf || len == 0) {
                reg->rax = 0;
                break;
            }

            stream_t* kbd = kbd_get_stream();
            ullong i = 0;

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
        case SYS_CLI_CLEAR: {
            dsp_clear();
            break;
        }
        case SYS_SET_COL: {
            dsp_set_color((ullong)reg->rdx, (ullong)reg->rdi);
            break;
        }
        case SYS_SET_CRSR: {
            dsp_set_cursor((ullong)reg->rdx, (ullong)reg->rdi);
            break;
        }
        case SYS_OPENDIR: {
            const char* path = (const char*)reg->rdx;
            reg->rax = (ullong)(long long)(path ? fat32_dir_open(path) : FAT_ERR_PARAM);
            break;
        }
        case SYS_READDIR: {
            int slot = (int)(long long)reg->rdx;
            AbiDirInfo* out = (AbiDirInfo*)reg->rdi;
            if(!out) { reg->rax = (ullong)(long long)FAT_ERR_PARAM; break; }
            DirInfo info;
            int rc = fat32_dir_read(slot, &info);
            if(rc != FAT_ERR_NONE) {
                reg->rax = rc;
                break;
            }
            uint n = info.name_len > ABI_NAME_MAX ? ABI_NAME_MAX : info.name_len;
            memset(out->name, 0, sizeof(out->name));
            memcpy(out->name, info.name, n);
            out->size     = info.size;
            out->attr     = info.attr;
            out->name_len = (uint)n;
            reg->rax = 0;
            break;
        }
        case SYS_CLOSEDIR: {
            int slot = (int)(long long)reg->rdx;
            reg->rax = (ullong)(long long)fat32_dir_close(slot);
            break;
        }
        case SYS_DELAY: {
            ullong ms = (ullong)reg->rdx;
            ullong t  = (ms * timer_hz() + 999) / 1000;

            reg->rax = 0;
           // thread_sleep_until(timer_ticks() + (t ? t : 1));
            break;
        }
        case SYS_START_PROC: {
            reg->rax = (ullong)(long long)start_proc((const char*)reg->rdx);
            break;
        }
        case SYS_FRAME_FLIP: {
            dsp_flip();
            reg->rax = 0;
            break;
        }
        default:
            reg->rax = -1;
            break;
    }
}
