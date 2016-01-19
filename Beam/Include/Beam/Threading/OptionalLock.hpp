#ifndef BEAM_OPTIONALLOCK_HPP
#define BEAM_OPTIONALLOCK_HPP
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/Threading.hpp"

namespace Beam {
namespace Threading {

  /*! \class OptionalLock
      \brief Acquires a mutex based on a template parameter.
      \tparam Acquire <code>true</code> iff the mutex should be acquired.
      \tparam MutexType The type of Mutex to acquire.
   */
  template<bool Acquire, typename MutexType = boost::mutex>
  class OptionalLock {};

  template<typename MutexType>
  class OptionalLock<true, MutexType> : private boost::noncopyable {
    public:

      //! Constructs an OptionalLock.
      /*!
        \param mutex The mutex to lock.
      */
      OptionalLock(MutexType& mutex);

    private:
      boost::lock_guard<MutexType> m_lock;
  };

  template<typename MutexType>
  class OptionalLock<false, MutexType> : private boost::noncopyable {
    public:

      //! Constructs an OptionalLock.
      /*!
        \param mutex The mutex to lock.
      */
      OptionalLock(MutexType& mutex);
  };

  template<typename MutexType>
  OptionalLock<true, MutexType>::OptionalLock(MutexType& mutex)
      : m_lock(mutex) {}

  template<typename MutexType>
  OptionalLock<false, MutexType>::OptionalLock(MutexType& mutex) {}
}
}

#endif
