#ifndef __FUTEX_H__
#define __FUTEX_H__

/**
 * A Mutex implementation utilizing Linux's futex system call
 * Basically Ulrich Drepper's "mutex2"/"mutex3" Mutex in modern CPP
 * Described in the paper "Futexes are tricky" (2004)
 * Author: Lucas Crämer
 * */

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

// helper functions
static inline int futex(std::uint32_t *uaddr, int futex_op, std::uint32_t val1,
                        const struct timespec *timeout, std::uint32_t *uaddr2,
                        std::uint32_t val3) {
  return syscall(SYS_futex, uaddr, futex_op, val1, timeout, uaddr2, val3);
}

static inline int futex_wait(std::uint32_t *uaddr, std::uint32_t expected_val) {
  return futex(uaddr, FUTEX_WAIT, expected_val, NULL, NULL, 0);
}

static inline int futex_wake(std::uint32_t *uaddr,
                             std::uint32_t threads_to_wakeup) {
  return futex(uaddr, FUTEX_WAKE, threads_to_wakeup, NULL, NULL, 0);
}

enum FutexState : std::uint32_t {
  UNLOCKED = 0,
  LOCKED_NO_WAITERS = 1,
  LOCKED_ONE_OR_MORE_WAITERS = 2,
};

class RawFutexLock {
private:
  std::atomic<FutexState> m_state;

public:
  constexpr RawFutexLock() noexcept : m_state(FutexState::UNLOCKED) {}

  // fast wait-free lock access try
  bool try_lock() noexcept {
    FutexState current_state = FutexState::UNLOCKED;
    return m_state.compare_exchange_strong(
        current_state, FutexState::LOCKED_NO_WAITERS, std::memory_order_relaxed,
        std::memory_order_acquire);
  }

  // lock algorithm from Drepper's mutex2
  void lock2() noexcept {
    FutexState current_state = FutexState::UNLOCKED;
    // IMPORTANT: acquire barrier when trying to access the lock
    // this implements the acquire-release protocol around the atomic within the
    // lock
    if (!m_state.compare_exchange_strong(
            current_state, FutexState::LOCKED_NO_WAITERS,
            std::memory_order_relaxed, std::memory_order_acquire)) {
      do {
        // if there are already one or more waiters
        // or if the lock is set with no waiters, set the state
        // to LOCKED_ONE_OR_MORE_WAITERS
        // if this fails the current state is either unlocked
        // or it is locked with one or more waiters
        // both cases require one CAS per loop iteration if there
        // is no spurious failure
        if (current_state == FutexState::LOCKED_ONE_OR_MORE_WAITERS ||
            m_state.compare_exchange_weak(
                (current_state = FutexState::LOCKED_NO_WAITERS),
                FutexState::LOCKED_ONE_OR_MORE_WAITERS,
                std::memory_order_relaxed, std::memory_order_relaxed)) {
          futex_wait((std::uint32_t *)&m_state,
                     FutexState::LOCKED_ONE_OR_MORE_WAITERS);
        }
        // hopefully the futex is unlocked now
        current_state = FutexState::UNLOCKED;
        // at this point either the thread was woken up or the futex was
        // unlocked or spurious failure try to acquire lock and set the new
        // state to LOCKED_ONE_OR_MORE_WAITERS since there is no certainty
        // whether other threads are (were) still waiting IMPORTANT: acquire
        // barrier again when accessing the atomic
      } while (!m_state.compare_exchange_weak(
          current_state, FutexState::LOCKED_ONE_OR_MORE_WAITERS,
          std::memory_order_relaxed, std::memory_order_acquire));
    }
  }

  // lock algorithm from Drepper's mutex3
  void lock3() noexcept {
    FutexState current_state = FutexState::UNLOCKED;
    // IMPORTANT: acquire barrier when trying to access the lock
    // this implements the acquire-release protocol around the atomic within the
    // lock
    if (!m_state.compare_exchange_strong(
            current_state, FutexState::LOCKED_NO_WAITERS,
            std::memory_order_relaxed, std::memory_order_acquire)) {
      // when the CAS fails this thread becomes a waiter
      // set the state to LOCKED_ONE_OR_MORE_WAITERS if thats not already the
      // case
      if (current_state != FutexState::LOCKED_ONE_OR_MORE_WAITERS)
        current_state = m_state.exchange(FutexState::LOCKED_ONE_OR_MORE_WAITERS,
                                         std::memory_order_relaxed);
      // the futex was possibly unlocked
      while (current_state != FutexState::UNLOCKED) {
        // futex is not unlocked call into futex syscall
        futex_wait((std::uint32_t *)&m_state,
                   FutexState::LOCKED_ONE_OR_MORE_WAITERS);
        // the thread was woken up and the futex is probably unlocked
        // try to acquire lock and set the new state to
        // LOCKED_ONE_OR_MORE_WAITERS since there is no certainty whether other
        // threads are (were) still waiting IMPORTANT: acquire barrier again
        // when accessing the atomic
        current_state = m_state.exchange(FutexState::LOCKED_ONE_OR_MORE_WAITERS,
                                         std::memory_order_acquire);
      }
    }
  }

  void unlock() noexcept {
    // IMPORTANT: release barrier to "publish" all previous non-atomic writes
    if (((std::atomic<std::uint32_t>)m_state)
            .fetch_sub(1, std::memory_order_release) !=
        FutexState::LOCKED_NO_WAITERS) {
      // when there are one or more waiters the threads should be woken up
      // and the subtraction did not do the trick yet to unlock the atomic
      m_state.store(FutexState::UNLOCKED, std::memory_order_relaxed);
      // wake up one thread in the kernel queue
      futex_wake((std::uint32_t *)&m_state, 1);
    }
  }
};

#endif