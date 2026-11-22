#include <stream.h>
#include <io.h>
#include <types.h>

int ringbuf_put(ringbuf_t* rb, uchar c) {
    uint next = (rb->head + 1) & (STREAM_BUFFER_SIZE - 1);
    if(next == rb->tail) return -1;

    rb->buf[rb->head] = c;
    rb->head = next;

    return 0;
}

int ringbuf_get(ringbuf_t* rb, uchar* out) {
    if(rb->tail == rb->head) return -1;
    *out = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) & (STREAM_BUFFER_SIZE - 1);

    return 0;
}

void stream_init(stream_t* s, uint flags) {
    s->rb.head = s->rb.tail = s->overflow = 0;
    s->flag = flags;
    s->read = stream_read;
    s->write = stream_write;
}

int stream_try_read(stream_t* s, void* dst, uint len) {
    if(!(s->flag & ST_READ) || !dst) return -1;

    uint n = 0;
    uchar* p = (uchar*)dst;
    uint f = irq_save();
    while (n < len && ringbuf_get(&s->rb, &p[n]) == 0) n++;
    irq_restore(f);

    return (int)n;
}

int stream_read(stream_t* s, void* dst, uint len) {
    uint got = 0;
    while(got < len) {
        int n = stream_try_read(s, (char*)dst + got, len - got);
        if(n > 0) got += (uint)n;
        else __asm__ volatile("sti; hlt");
    }

    return (int)got;
}

int stream_write(stream_t* s, const void* src, uint len) {
    if(!(s->flag & ST_WRITE) || !src) return -1;

    const uchar* p = (const uchar*)src;
    for(uint i = 0; i < len; ++i) {
        while(ringbuf_put(&s->rb, p[i]) != 0) {
            s->overflow++;
            __asm__ volatile("sti; hlt");
        }
    }
    return (int)len;
}
