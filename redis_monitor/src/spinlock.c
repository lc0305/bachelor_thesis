#include "spinlock.h"

static inline bool atomic_compare_exchange_weak(int *ptr, int compare,
                                                int exchange) {
  return __atomic_compare_exchange_n(ptr, &compare, exchange, 0,
                                     __ATOMIC_RELAXED, __ATOMIC_ACQUIRE);
}

static inline void atomic_store(int *ptr, int value) {
  __atomic_store_n(ptr, value, __ATOMIC_RELEASE);
}

void spinlock_lock(SpinLock *spinlock) {
  while (!atomic_compare_exchange_weak(&spinlock->locked, 0, 1))
    ;
}

void spinlock_unlock(SpinLock *spinlock) {
  atomic_store(&spinlock->locked, 0);
}
