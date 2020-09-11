#ifndef BEAM_LOCK_RELEASE_HPP
#define BEAM_LOCK_RELEASE_HPP
#include "Beam/Threading/Threading.hpp"

namespace Beam::Threading {

  /**
   * Releases a lock, locking it again upon destruction.
   * @param <L> The type of lock to release.
   */
  template<typename L>
  class LockRelease {
    public:

      /** The type of lock to release. */
      using Lock = L;

      /**
       * Constructs a LockRelease, invoking a lock's unlock method.
       * @param lock The lock to release.
       */
      LockRelease(Lock& lock);

      /**
       * Acquires a LockRelease.
       * @param lockRelease The LockRelease to acquire.
       */
      LockRelease(LockRelease&& lockRelease);

      ~LockRelease();

      /** Acquires the lock. */
      void Acquire();

      /** Releases the lock. */
      void Release();

    private:
      Lock* m_lock;
      bool m_isReleased;
  };

  /**
   * Builds a LockRelease from a lock.
   * @param lock The lock to release.
   * @return An instance that releases the lock.
   */
  template<typename Lock>
  LockRelease<Lock> Release(Lock& lock) {
    return LockRelease(lock);
  }

  template<typename L>
  LockRelease<L>::LockRelease(Lock& lock)
      : m_lock(&lock),
        m_isReleased(true) {
    lock.unlock();
  }

  template<typename L>
  LockRelease<L>::LockRelease(LockRelease&& lockRelease)
      : m_lock(lockRelease.m_lock),
        m_isReleased(lockRelease.m_isReleased) {
    lockRelease.m_isReleased = false;
  }

  template<typename L>
  LockRelease<L>::~LockRelease() {
    if(m_isReleased) {
      m_lock->lock();
    }
  }

  template<typename L>
  void LockRelease<L>::Acquire() {
    if(m_isReleased) {
      m_lock->lock();
      m_isReleased = false;
    }
  }

  template<typename L>
  void LockRelease<L>::Release() {
    if(m_isReleased) {
      return;
    }
    m_lock->unlock();
    m_isReleased = true;
  }
}

#endif
