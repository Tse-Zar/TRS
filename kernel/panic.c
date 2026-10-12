/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = Unified kernel panic system ------ =
    ======================================
*/

#include <panic.h>
#include <image.h>
#include <idt.h>

static int in_panic = 0;

static void out_dec(unsigned long long v) {
    char buf[21];
    int i = 20;

    buf[i] = '\0';
    if(v == 0) buf[--i] = '0';
    while(v > 0) {
        buf[--i] = (char)('0' + (v % 10));
        v /= 10;
    }

    dsp_print(&buf[i]);
}

static void field(const char* name, unsigned long long v) {
    dsp_print(name);
    dsp_print(" = ");
    dsp_print_hex(v);
}

void panic_halt(void) {
    while(1) {
        __asm__ volatile("cli; hlt");
    }
}

static void panic_header(const char* title) {
    if(in_panic) panic_halt();
    in_panic = 1;

    __asm__ volatile("cli");
    dsp_set_color(_PanicRed, _PanicDark);
    dsp_clear();
    dsp_print("\n\n << TRS PANIC >>\n");
    dsp_print(title);
    dsp_print("\n");
}

void kpanic(const char *msg, const char *file, int line) {
    panic_header("--- Kernel panic ---");

    dsp_print("  reason: ");
    dsp_print(msg ? msg : "(no message)");
    dsp_print("\n at ");
    dsp_print(file ? file : "???");
    dsp_print(" : ");
    out_dec((unsigned long long)line);
    dsp_print("\n\nSystem halted. Reset required\n");

    panic_halt();
}

void kpanic_regs(const _regs_t *reg, const char *msg) {
    panic_header("--- CPU exception ---");

    dsp_print("  ");
    dsp_print(msg ? msg : "unknown exception");
    dsp_print("\n");

    dsp_print("\n  "); field("vector", reg->int_no);
    dsp_print("\n  "); field("err", reg->err_code);
    dsp_print("\n");

    dsp_print("\n  "); field("rip", reg->rip);
    dsp_print("\n  "); field("cs", reg->cs);
    dsp_print("\n  "); field("rflags", reg->rflags);
    
    dsp_print("\n  "); field("ss", reg->ss);
    dsp_print("\n  "); field("rsp", reg->rsp);
    
    if(reg->int_no == 14) {
        unsigned long long cr2;
        __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
        dsp_print("\n  "); field("cr2", cr2);
    }
    dsp_print("\n");
    dsp_print("=== registers ===\n");

    dsp_print("  "); field("rax", reg->rax);  dsp_print("\n");
    dsp_print("  "); field("rcx", reg->rcx);  dsp_print("\n");
    dsp_print("  "); field("rsi", reg->rsi);  dsp_print("\n");
    dsp_print("  "); field("rbp", reg->rbp);  dsp_print("\n");
    dsp_print("  "); field("rbx", reg->rbx);  dsp_print("\n");
    dsp_print("  "); field("rdx", reg->rdi);  dsp_print("\n");
    dsp_print("  "); field("rdi", reg->rdi);  dsp_print("\n");
    dsp_print("  "); field("r8", reg->r8);    dsp_print("\n");
    dsp_print("  "); field("r9", reg->r9);    dsp_print("\n");
    dsp_print("  "); field("r10", reg->r10);  dsp_print("\n");
    dsp_print("  "); field("r11", reg->r11);  dsp_print("\n");
    dsp_print("  "); field("r12", reg->r12);  dsp_print("\n");
    dsp_print("  "); field("r13", reg->r13);  dsp_print("\n");
    dsp_print("  "); field("r14", reg->r14);  dsp_print("\n");
    dsp_print("  "); field("r15", reg->r15);  dsp_print("\n");

    dsp_print("\n\nSystem halted. Reset required.\n");

    panic_halt();
}

void kassert_fail(const char *expr, const char *file, int line) {
    panic_header("Assertion failed");

    dsp_print("  ");
    dsp_print(expr ? expr : "(null)");
    dsp_print("\n at");
    dsp_print(file ? file : "???");
    dsp_print(" : ");
    out_dec((unsigned long long)line);
    dsp_print("\n\nSystem halted. Reset required.\n");

    panic_halt();
}