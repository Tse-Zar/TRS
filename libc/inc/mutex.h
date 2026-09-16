#ifndef MUTEX_H
#define MUTEX_H

typedef struct {
    unsigned char locked;
} mutex;

void mutex_lock(mutex* m);
void mutex_unlock(mutex* m);

#endif // MUTEX_H