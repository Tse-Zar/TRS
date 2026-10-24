#ifndef PROCESS_H
#define PROCESS_H

typedef enum {
    NULL
} process_type;

typedef struct {
    process_type type;
    const char*  name;
    unsigned int pid;
} procthread;

#endif // PROCESS_H