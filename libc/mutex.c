#include <mutex.h>

void mutex_lock(mutex *m) {
    while(m->locked) ;
    m->locked = 1;
}

void mutex_unlock(mutex *m) {
    m->locked = 0;
}