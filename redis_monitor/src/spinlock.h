#include <pthread.h>
#include <stdbool.h>

typedef struct SpinLock {
    int locked;
} SpinLock;

#define SPINLOCK_INIT { 0 };

void spinlock_lock(SpinLock *spinlock);
void spinlock_unlock(SpinLock *spinlock);
