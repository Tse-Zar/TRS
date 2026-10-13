#ifndef MUTEX_H
#define MUTEX_H

typedef struct {
    volatile unsigned char locked;
} mutex;

typedef  struct {
    volatile unsigned char locked;
} spinlock_t;

void mutex_lock(mutex* m);
int mutex_trylock(mutex* m);
void mutex_unlock(mutex* m);

void spin_lock(spinlock_t* s);
int spin_trylock(spinlock_t* s);
void spin_unlock(spinlock_t* s);

#endif // MUTEX_H