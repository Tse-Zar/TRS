#ifndef STREAM_H
#define STREAM_H

#include <std.h>
#include <types.h>

#define STREAM_BUFFER_SIZE 4096

typedef struct {
    uchar buf[STREAM_BUFFER_SIZE];
    volatile uint head;
    volatile uint tail;
} ringbuf_t;

typedef enum {
    ST_READ = 1,
    ST_WRITE = 2
} stream_flags_t;

typedef struct stream{
    ringbuf_t rb;
    volatile uint flag;
    volatile uint overflow;
    int (*write)(struct stream* s, const void* dst, uint len);
    int (*read)(struct stream* s, void* src, uint len);
} stream_t;

int ringbuf_put(ringbuf_t* rb, uchar c);
int ringbuf_get(ringbuf_t* rb, uchar* out);
int stream_read(stream_t* s, void* dst, uint len);
int stream_try_read(stream_t* s, void* dst, uint len);
int stream_write(stream_t* s, const void* src, uint len);
void stream_init(stream_t* s, uint flags);

#endif // STREAM_H
