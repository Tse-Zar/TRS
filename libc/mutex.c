#include <mutex.h>

void mspin_lock(spinlock_t *s) {
    while(__atomic_exchange_n(&s->locked, 1, __ATOMIC_ACQUIRE)) __asm__ volatile("pause" ::: "memory");
}

int spin_trylock(spinlock_t *s) {
    return __atomic_exchange_n(&s->locked, 1, __ATOMIC_ACQUIRE) == 0;
}

void spin_unlock(spinlock_t *s) {
    __atomic_store_n(&s->locked, 0, __ATOMIC_RELEASE);
}

void mutex_lock(mutex *m) {
    
}