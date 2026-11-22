#ifndef MUTEX_H
#define MUTEX_H

#include <types.h>

typedef struct {
    volatile uchar locked;
} mutex;

typedef  struct {
    volatile uchar locked;
} spinlock_t;

void mutex_lock(mutex* m);
int mutex_trylock(mutex* m);
void mutex_unlock(mutex* m);

void spin_lock(spinlock_t* s);
int spin_trylock(spinlock_t* s);
void spin_unlock(spinlock_t* s);

#endif // MUTEX_H
