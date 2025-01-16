#ifndef BEAM_THREADING_SPIN_MUTEX_HPP
#define BEAM_THREADING_SPIN_MUTEX_HPP
#include <atomic>
#ifdef _MSC_VER
  #include <intrin.h>
#else
  #include <emmintrin.h>
#endif
#include "Beam/Threading/Threading.hpp"

namespace Beam::Threading {

  /**
   * @brief A simple spin mutex implementation.
   *
   * This spin mutex uses an atomic_flag to provide a busy-wait
   * locking mechanism. It is improved by using a CPU pause instruction
   * during spin-wait loops to reduce contention, which is particularly
   * beneficial on hyper-threaded processors.
   *
   * @note This mutex is not recursive.
   */
  class SpinMutex {
    public:

      /**
       * @brief Default constructor.
       *
       * Initializes the spin mutex in an unlocked state.
       */
      SpinMutex() noexcept = default;

      /**
       * @brief Acquires the lock.
       *
       * Spins until the lock is acquired. Uses a CPU pause instruction
       * within the loop to reduce resource contention.
       */
      void lock() noexcept;

      /**
       * @brief Releases the lock.
       *
       * Clears the atomic flag so that other threads can acquire the lock.
       */
      void unlock() noexcept;

      /**
       * @brief Attempts to acquire the lock without blocking.
       *
       * @return true if the lock was successfully acquired, false otherwise.
       */
      bool try_lock() noexcept;

    private:
      std::atomic_flag m_flag;

      SpinMutex(const SpinMutex&) = delete;
      SpinMutex& operator =(const SpinMutex&) = delete;
  };

  inline void SpinMutex::lock() noexcept {
    while(m_flag.test_and_set(std::memory_order_acquire)) {
      #ifdef _MSC_VER
        _mm_pause();
      #else
        __builtin_ia32_pause();
      #endif
    }
  }

  inline void SpinMutex::unlock() noexcept {
    m_flag.clear(std::memory_order_release);
  }

  inline bool SpinMutex::try_lock() noexcept {
    return !m_flag.test_and_set(std::memory_order_acquire);
  }
}

#endif
