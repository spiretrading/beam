#ifndef BEAM_LOCK_RELEASE_HPP
#define BEAM_LOCK_RELEASE_HPP
#include <concepts>
#include <utility>

namespace Beam {

  /** Concept satisfied if a type can be locked and unlocked. */
  template<typename T>
  concept IsBasicLockable = requires(T t) {
    { t.lock() } -> std::same_as<void>;
    { t.unlock() } -> std::same_as<void>;
  };

  /**
   * Releases a lock, locking it again upon destruction.
   * @tparam L The type of lock to release.
   */
  template<IsBasicLockable L>
  class LockRelease {
    public:

      /** The type of lock to release. */
      using Lock = L;

      /**
       * Constructs a LockRelease, invoking a lock's unlock method.
       * @param lock The lock to release.
       */
      LockRelease(Lock& lock) noexcept;

      LockRelease(LockRelease&& release) noexcept;
      ~LockRelease();

      /** Acquires the lock. */
      void acquire();

      /** Releases the lock. */
      void release();

    private:
      Lock* m_lock;
      bool m_is_released;

      LockRelease(const LockRelease&) = delete;
      LockRelease& operator =(const LockRelease&) = delete;
  };

  /**
   * Returns a LockRelease from a lock.
   * @param lock The lock to release.
   * @return An instance that releases the lock.
   */
  template<IsBasicLockable L>
  LockRelease<L> release(L& lock) {
    return LockRelease(lock);
  }

  template<IsBasicLockable L>
  LockRelease<L>::LockRelease(Lock& lock) noexcept
      : m_lock(&lock),
        m_is_released(true) {
    lock.unlock();
  }

  template<IsBasicLockable L>
  LockRelease<L>::LockRelease(LockRelease&& release) noexcept
    : m_lock(release.m_lock),
      m_is_released(std::exchange(release.m_is_released, false)) {}

  template<IsBasicLockable L>
  LockRelease<L>::~LockRelease() {
    if(m_is_released) {
      m_lock->lock();
    }
  }

  template<IsBasicLockable L>
  void LockRelease<L>::acquire() {
    if(m_is_released) {
      m_lock->lock();
      m_is_released = false;
    }
  }

  template<IsBasicLockable L>
  void LockRelease<L>::release() {
    if(m_is_released) {
      return;
    }
    m_lock->unlock();
    m_is_released = true;
  }
}

#endif
