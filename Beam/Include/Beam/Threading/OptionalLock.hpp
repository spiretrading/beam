#ifndef BEAM_OPTIONAL_LOCK_HPP
#define BEAM_OPTIONAL_LOCK_HPP
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/Threading.hpp"

namespace Beam::Threading {

  /**
   * Acquires a mutex based on a template parameter.
   * @param <A> <code>true</code> iff the mutex should be acquired.
   * @param <M> The type of mutex to acquire.
   */
  template<bool A, typename M = boost::mutex>
  class OptionalLock {};

  template<typename M>
  class OptionalLock<true, M> {
    public:

      /** The type of mutex to acquire. */
      using Mutex = M;

      /**
       * Constructs an OptionalLock.
       * @param mutex The mutex to lock.
       */
      OptionalLock(M& mutex);

    private:
      boost::lock_guard<Mutex> m_lock;

      OptionalLock(const OptionalLock&) = delete;
      OptionalLock& operator =(const OptionalLock&) = delete;
  };

  template<typename M>
  class OptionalLock<false, M> {
    public:

      /** The type of mutex to acquire. */
      using Mutex = M;

      /**
       * Constructs an OptionalLock.
       * @param mutex The mutex to lock.
       */
      OptionalLock(Mutex& mutex);

    private:
      OptionalLock(const OptionalLock&) = delete;
      OptionalLock& operator =(const OptionalLock&) = delete;
  };

  template<typename M>
  OptionalLock<true, M>::OptionalLock(Mutex& mutex)
    : m_lock(mutex) {}

  template<typename M>
  OptionalLock<false, M>::OptionalLock(Mutex& mutex) {}
}

#endif
