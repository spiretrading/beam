#ifndef BEAM_LOCKRELEASE_HPP
#define BEAM_LOCKRELEASE_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Threading/Threading.hpp"

namespace Beam {
namespace Threading {

  /*! \class LockRelease
      \brief Releases a lock, locking it again upon destruction.
   */
  template<typename LockType>
  class LockRelease : private boost::noncopyable {
    public:

      //! The type of lock to release.
      using Lock = LockType;

      //! Constructs a LockRelease, invoking a lock's unlock method.
      /*!
        \param lock The lock to release.
      */
      LockRelease(Lock& lock);

      //! Acquires a LockRelease.
      /*!
        \param lockRelease The LockRelease to acquire.
      */
      LockRelease(LockRelease&& lockRelease);

      ~LockRelease();

      //! Acquires the lock.
      void Acquire();

      //! Releases the lock.
      void Release();

    private:
      Lock* m_lock;
      bool m_isReleased;
  };

  //! Builds a LockRelease from a lock.
  /*!
    \param lock The lock to release.
    \return An instance that releases the lock.
  */
  template<typename LockType>
  LockRelease<LockType> Release(LockType& lock) {
    return LockRelease<LockType>(lock);
  }

  template<typename LockType>
  LockRelease<LockType>::LockRelease(Lock& lock)
      : m_lock(&lock),
        m_isReleased(true) {
    lock.unlock();
  }

  template<typename LockType>
  LockRelease<LockType>::LockRelease(LockRelease&& lockRelease)
      : m_lock(lockRelease.m_lock),
        m_isReleased(lockRelease.m_isReleased) {
    lockRelease.m_isReleased = false;
  }

  template<typename LockType>
  LockRelease<LockType>::~LockRelease() {
    if(m_isReleased) {
      m_lock->lock();
    }
  }

  template<typename LockType>
  void LockRelease<LockType>::Acquire() {
    if(m_isReleased) {
      m_lock->lock();
      m_isReleased = false;
    }
  }

  template<typename LockType>
  void LockRelease<LockType>::Release() {
    if(m_isReleased) {
      return;
    }
    m_lock->unlock();
    m_isReleased = true;
  }
}
}

#endif
