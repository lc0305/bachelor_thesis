#ifndef __SPSC_RING_H__
#define __SPSC_RING_H__

/**
 * Basically Herlihy's WaitFreeQueue<T> in modern CPP
 * Author: Lucas Crämer
 * */

#include <array>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <optional>

template <typename T, std::size_t N> class SPSCRing {
private:
  std::atomic<std::size_t> m_head;
  std::array<T, N> m_buff;
  std::atomic<std::size_t> m_tail;

public:
  // initialize head and tail
  constexpr SPSCRing() noexcept : m_head(0), m_tail(0) {}

  // return ring size
  constexpr std::size_t size() noexcept { return N; }

  bool enqueue(T val) noexcept {
    std::size_t current_tail = m_tail.load(std::memory_order_relaxed);
    if (current_tail - m_head.load(std::memory_order_relaxed) < N) {
      // The queue is not full.
      // Insert the element at the current tail index.
      m_buff[current_tail % N] = std::move(val);
      // (possible optimization to get rid off the modulus, N = 2^x, index =
      // current_tail & (N - 1)) Increment the tail. Do not reorder any read or
      // writes after the increment. (Before means before!) 'Release' the
      // previous writes.
      // <----------------------------------------------------------------------->
      // compiler and memory barrier
      m_tail.store(++current_tail, std::memory_order_release);
      // Success
      return true;
    }
    // The queue is full. Return false.
    return false;
  }

  std::optional<T> dequeue() noexcept {
    // Load the tail.
    // Do not reorder any read or writes before the load. (After means after!)
    // Acquire barrier makes writes before the release store of the atomic
    // variable visible.
    const std::size_t current_tail = m_tail.load(std::memory_order_acquire);
    // <----------------------------------------------------------------------->
    // compiler and memory barrier
    std::size_t current_head = m_head.load(std::memory_order_relaxed);
    // Is the buffer not empty?
    if (current_head != current_tail) {
      auto value = std::move(m_buff[current_head % N]);
      m_head.store(++current_head, std::memory_order_relaxed);
      return std::optional(std::move(value));
    }
    // Else return NULL.
    return std::nullopt;
  }

  // for debugging only
  void debug_print_tail() {
    std::cout << "tail: " << m_tail.load(std::memory_order_relaxed)
              << std::endl;
  }

  // for debugging only
  void debug_print_head() {
    std::cout << "head: " << m_head.load(std::memory_order_relaxed)
              << std::endl;
  }
};

#endif
