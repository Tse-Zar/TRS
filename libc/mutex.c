#include <mutex.h>

void mutex_lock(mutex *m) {
    while(__atomic_exchange_n(&m->locked, 1, __ATOMIC_ACQUIRE)) __asm__ volatile("pause" ::: "memory");
}

int mutex_trylock(mutex *m) {
    return __atomic_exchange_n(&m->locked, 1, __ATOMIC_ACQUIRE) == 0;
}

void mutex_unlock(mutex *m) {
    __atomic_store_n(&m->locked, 0, __ATOMIC_RELEASE);
}