#ifndef __MPMC_RING_H__
#define __MPMC_RING_H__

/**
 * Inspired by 'An Interesting Lock-free Queue'
 * Talk by Tony Van Eerd at the CppCon 2017:
 * https://www.youtube.com/watch?v=HP2InVqgBFM
 * Author: Lucas Crämer
 * */

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <optional>

// This is the entry structure for the buffer.
// It holds a pointer to the actual value and a generation for that pointer.
template <typename T> struct PtrGen {
  std::optional<T *> ptr;
  std::uint64_t gen;

  inline bool is_empty(std::uint64_t cmp_gen) noexcept {
    // The entry is empty if the ptr points to NULL and it is in the current
    // gen.
    return !ptr.has_value() && gen == cmp_gen;
  }

  inline bool is_valid(std::uint64_t cmp_gen) noexcept {
    // The entry is valid if the ptr does not point to NULL and it is in the
    // current gen.
    return ptr.has_value() && gen == cmp_gen;
  }
};

// This is the data structure for the head and tail of the queue.
// It holds an index for head/tail into the buffer (this index is likely close
// the real index) and the the generation of that index to mitigate the ABA
// problem.
template <std::size_t N> struct IdxGen {
  std::size_t idx;
  std::uint64_t gen;

  inline void incr() noexcept {
    // Increment the index, if the index reaches the buffer size start at 0
    // again and increment the generation.
    if (++idx == N) {
      idx = 0;
      ++gen;
    }
  }

  inline bool operator<(const IdxGen<N> &rhs) noexcept {
    return gen < rhs.gen || (gen == rhs.gen && idx < rhs.idx);
  }
};

// Multiple-producer multiple-consumer circular FIFO queue
// Holds pointer to a type T.
// N is the cacapcity of the queue.
template <typename T, std::size_t N> class MPMCRing {
private:
  std::atomic<IdxGen<N>> m_headish;
  // The buff in between head and tail, so that depending on the buff size they
  // will likely end up on a different cache line. (no contention on the cache
  // line)
  std::array<std::atomic<PtrGen<T>>, N> m_buff;
  std::atomic<IdxGen<N>> m_tailish;

public:
  constexpr MPMCRing() noexcept
      : m_headish(IdxGen<N>{0, 0}), m_tailish(IdxGen<N>{0, 0}) {
    // Start by zeroing the buffer.
    std::fill(std::begin(m_buff), std::end(m_buff), PtrGen<T>{std::nullopt, 0});
  }

  // return ring size
  constexpr std::size_t size() noexcept { return N; }

  // Enqueue a pointer to a type T.
  // Returns true on success.
  // Returns false if the queue is full and enqueueing was not successful.
  bool enqueue(T *val) noexcept {
    std::uint64_t prev_gen = 0;
    IdxGen<N> old_tailish;
    // Load the 'current tail'. This tail is just a hint and does not have to be
    // the 'real' tail.
    // No memory barrier required due to acquire-release CAS (I think).
    IdxGen<N> new_tailish = old_tailish =
        m_tailish.load(std::memory_order_relaxed);
    PtrGen<T> current_entry;
    do {
      // Iterate over the buffer while the entry is not empty. No memory fencing
      // required.
      while (!(current_entry =
                   m_buff[new_tailish.idx].load(std::memory_order_relaxed))
                  .is_empty(new_tailish.gen)) {
        // The entry is not empty because the pointer is either not NULL or not
        // in the same generation as the current tail.
        if (current_entry.gen < prev_gen) {
          // The buffer is full, because the entry is not empty and the
          // generation of the current entry has decreased. Update the tail if
          // its smaller and return.
          if (old_tailish < new_tailish) {
            // Update the tail if its smaller and return.
            // Strong CAS, because not in a loop.
            // No writes, no memory barrier required.
            m_tailish.compare_exchange_strong(old_tailish, new_tailish,
                                              std::memory_order_relaxed,
                                              std::memory_order_relaxed);
          }
          return false;
        }
        // Increment the tail for further iteration.
        new_tailish.incr();
        // Replace the previous generation with the generation of the current
        // entry if the pointer points to a valid address.
        if (current_entry.ptr.has_value())
          prev_gen = current_entry.gen;
      }
      // Finally: Try adding the entry to the buffer. If another thread changed
      // the entry retry. Weak CAS, because loop.
      // Do not reorder any read or writes before/after the successful CAS.
    } while (!m_buff[new_tailish.idx].compare_exchange_weak(
        current_entry, PtrGen<T>{val, new_tailish.gen},
        std::memory_order_release, std::memory_order_acquire));
    // Increment the tail, a new value was added.
    new_tailish.incr();
    // Update the tail.
    // Strong CAS, because not in a loop.
    // No memory barrier required due to acquire-release CAS (I think).
    m_tailish.compare_exchange_strong(old_tailish, new_tailish,
                                      std::memory_order_relaxed,
                                      std::memory_order_relaxed);
    // Success.
    return true;
  }

  // Returns an optional pointer to a value T. If the queue is empty return
  // NULL.
  std::optional<T *> dequeue() noexcept {
    IdxGen<N> old_headish;
    // Load the 'current head'. This head is just a hint and does not have to be
    // the 'real' head.
    // No memory barrier required due to acquire-release  CAS (I think).
    IdxGen<N> new_headish = old_headish =
        m_headish.load(std::memory_order_relaxed);
    PtrGen<T> current_entry;

    do {
      // Iterate over the buffer while the entry is not valid. No memory fencing
      // required.
      while (!(current_entry =
                   m_buff[new_headish.idx].load(std::memory_order_relaxed))
                  .is_valid(new_headish.gen)) {
        // The entry is not valid because the pointer is either NULL or not in
        // the same generation as the current head.
        if (current_entry.gen == new_headish.gen) {
          // If the entry is in the same generation as the current head the
          // queue is empty.
          if (old_headish < new_headish) {
            // Update the head if its smaller and return.
            // Strong CAS, because not in a loop.
            // No writes, no memory barrier required.
            m_headish.compare_exchange_strong(old_headish, new_headish,
                                              std::memory_order_relaxed,
                                              std::memory_order_relaxed);
          }
          return std::nullopt;
        }
        // Increment the head for further iteration.
        new_headish.incr();
      }
      // Finally: Try removing the entry from the buffer. If another thread
      // changed the entry retry. Weak CAS, because loop.
      // Do not reorder any read or writes before/after the successful CAS.
    } while (!m_buff[new_headish.idx].compare_exchange_weak(
        current_entry, PtrGen<T>{std::nullopt, (new_headish.gen + 1)},
        std::memory_order_release, std::memory_order_acquire));
    // Increment the head, a new value was removed from the buffer.
    new_headish.incr();
    // Update the head.
    // Strong CAS, because not in a loop.
    // No memory barrier required due to acquire-release CAS (I think).
    m_headish.compare_exchange_strong(old_headish, new_headish,
                                      std::memory_order_relaxed,
                                      std::memory_order_relaxed);
    // Success. Return the pointer.
    return current_entry.ptr;
  }

  // for debugging only
  void debug_print_tail() {
    auto tail = m_tailish.load(std::memory_order_relaxed);
    std::cout << "tail idx: " << tail.idx << ", gen: " << tail.gen << std::endl;
  }

  // for debugging only
  void debug_print_head() {
    auto head = m_headish.load(std::memory_order_relaxed);
    std::cout << "head idx: " << head.idx << ", gen: " << head.gen << std::endl;
  }
};

#endif
