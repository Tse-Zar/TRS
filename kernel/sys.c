#include <sys.h>
#include <image.h>
#include <keyboard.h>
#include <stream.h>
#include <fat32.h>
#include <string.h>

#define ABI_NAME_MAX 32

typedef struct {
    char          name[ABI_NAME_MAX + 1];
    unsigned int  size;
    unsigned char attr;
    unsigned char name_len;
} AbiDirInfo;

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
        case SYS_CLI_CLEAR: {
            dsp_clear();
            break;
        }
        case SYS_SET_COL: {
            dsp_set_color((unsigned long long)reg->rdx, (unsigned long long)reg->rdi);
            break;
        }
        case SYS_SET_CRSR: {
            dsp_set_cursor((unsigned long long)reg->rdx, (unsigned long long)reg->rdi);
            break;
        }
        case SYS_OPENDIR: {
            const char* path = (const char*)reg->rdx;
            reg->rax = (unsigned long long)(long long)(path ? fat32_dir_open(path) : FAT_ERR_PARAM);
            break;
        }
        case SYS_READDIR: {
            int slot = (int)(long long)reg->rdx;
            AbiDirInfo* out = (AbiDirInfo*)reg->rdi;
            if(!out) { reg->rax = (unsigned long long)(long long)FAT_ERR_PARAM; break; }
            DirInfo info;
            int rc = fat32_dir_read(slot, &info);
            if(rc != FAT_ERR_NONE) {
                reg->rax = rc;
                break;
            }
            unsigned int n = info.name_len > ABI_NAME_MAX ? ABI_NAME_MAX : info.name_len;
            memset(out->name, 0, sizeof(out->name));
            memcpy(out->name, info.name, n);
            out->size     = info.size;
            out->attr     = info.attr;
            out->name_len = (unsigned int)n;
            reg->rax = 0;
            break;
        }
        case SYS_CLOSEDIR: {
            int slot = (int)(long long)reg->rdx;
            reg->rax = (unsigned long long)(long long)fat32_dir_close(slot);
            break;
        }
        default:
            reg->rax = -1;
            break;
    }
}