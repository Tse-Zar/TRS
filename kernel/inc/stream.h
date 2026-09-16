#ifndef STREAM_H
#define STREAM_H

#include <std.h>

#define STREAM_BUFFER_SIZE 256

typedef struct {
    unsigned char buf[STREAM_BUFFER_SIZE];
    volatile unsigned int head;
    volatile unsigned int tail;
} ringbuf_t;

typedef enum {
    ST_READ = 1,
    ST_WRITE = 2
} stream_flags_t;

typedef struct stream{
    ringbuf_t rb;
    volatile unsigned flag;
    volatile unsigned overflow;
    int (*write)(struct stream* s, const void* dst, unsigned len);
    int (*read)(struct stream* s, void* src, unsigned len);
} stream_t;

int ringbuf_put(ringbuf_t* rb, unsigned char c);
int ringbuf_get(ringbuf_t* rb, unsigned char* out);
int stream_read(stream_t* s, void* dst, unsigned len);
int stream_try_read(stream_t* s, void* dst, unsigned len);
int stream_write(stream_t* s, const void* src, unsigned len);
void stream_init(stream_t* s, unsigned flags);

extern stream_t* kstdin, *kstdout, *kstderr;

#endif // STREAM_H