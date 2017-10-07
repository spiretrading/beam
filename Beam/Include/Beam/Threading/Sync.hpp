#ifndef BEAM_SYNC_HPP
#define BEAM_SYNC_HPP
#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/utility/declval.hpp>
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Threading {

  /*! \class Sync
      \brief Synchronizes access to a resource.
      \tparam T The type of value to store.
      \tparam MutexType The type of mutex to use.
   */
  BEAM_SUPPRESS_MULTIPLE_CONSTRUCTORS()
  template<typename T, typename MutexType = boost::mutex>
  class Sync : private boost::noncopyable {
    public:

      //! The value type stored.
      using Value = T;

      //! The type of mutex used.
      using Mutex = MutexType;

      //! Constructs a Sync.
      /*!
        \param args The parameters to pass to the synchronized value.
      */
      template<typename... Args>
      Sync(Args&&... args);

      //! Copies a Sync.
      /*!
        \param sync The Sync to copy.
      */
      Sync(const Sync& sync);

      //! Copies a Sync.
      /*!
        \param sync The Sync to copy.
      */
      Sync(Sync& sync);

      //! Moves a Sync.
      /*!
        \param sync The Sync to move.
      */
      Sync(Sync&& sync);

      //! Returns a reference of the value.
      T& AcquireReference();

      //! Returns a copy of the value.
      T Acquire() const;

      template<typename F>
      decltype(auto) With(F f) {
        boost::unique_lock<Mutex> lock(m_mutex);
        m_lock = &lock;
        return f(m_value);
      }

      template<typename F>
      decltype(auto) With(F f) const {
        boost::unique_lock<Mutex> lock(m_mutex);
        m_lock = &lock;
        return f(m_value);
      }

      template<typename S2, typename M2, typename F>
      decltype(auto) With(Sync<S2, M2>& s2, F f) {
        boost::unique_lock<Mutex> lock1(m_mutex, boost::defer_lock);
        boost::unique_lock<M2> lock2(s2.m_mutex, boost::defer_lock);
        boost::lock(lock1, lock2);
        m_lock = &lock1;
        s2.m_lock = &lock2;
        return f(m_value, s2.m_value);
      }

      //! Returns the lock used for synchronization.
      boost::unique_lock<Mutex>& GetLock() const;

      //! Assigns a Sync.
      /*!
        \param sync The Sync to assign from.
        \return A reference to <i>*this</i>.
      */
      template<typename U>
      Sync& operator =(const Sync<U>& sync);

      //! Assigns a Sync.
      /*!
        \param sync The Sync to assign from.
        \return A reference to <i>*this</i>.
      */
      template<typename U>
      Sync& operator =(Sync<U>&& sync);

      //! Assigns a value.
      /*!
        \param value The value to assign.
      */
      template<typename ValueForward>
      Sync& operator =(ValueForward&& value);

    private:
      template<typename, typename> friend class Sync;
      template<typename S1, typename M1>
        friend LockRelease<boost::unique_lock<typename Sync<S1, M1>::Mutex>>
        Release(Sync<S1, M1>&);
      mutable Mutex m_mutex;
      mutable boost::unique_lock<Mutex>* m_lock;
      T m_value;
  };
  BEAM_UNSUPPRESS_MULTIPLE_CONSTRUCTORS()

  //! Acquires a Sync's instance in a synchronized manner.
  /*!
    \param sync The Sync whose instance is to be acquired.
    \param f The action to perform on the instance.
  */
  template<typename S1, typename M1, typename F>
  decltype(auto) With(Sync<S1, M1>& s1, F f) {
    return s1.With(f);
  }

  //! Acquires a Sync's instance in a synchronized manner.
  /*!
    \param sync The Sync whose instance is to be acquired.
    \param f The action to perform on the instance.
  */
  template<typename S1, typename M1, typename F>
  decltype(auto) With(const Sync<S1, M1>& s1, F f) {
    return s1.With(f);
  }

  template<typename S1, typename M1, typename S2, typename M2, typename F>
  decltype(auto) With(Sync<S1, M1>& s1, Sync<S2, M2>& s2, F f) {
    return s1.With(s2, f);
  }

  //! Releases a Sync.
  template<typename S1, typename M1>
  LockRelease<boost::unique_lock<typename Sync<S1, M1>::Mutex>> Release(
      Sync<S1, M1>& s1) {
    return LockRelease<boost::unique_lock<typename Sync<S1, M1>::Mutex>>(
      *s1.m_lock);
  }

  template<typename T, typename MutexType>
  template<typename... Args>
  Sync<T, MutexType>::Sync(Args&&... args)
      : m_value(std::forward<Args>(args)...) {}

  template<typename T, typename MutexType>
  Sync<T, MutexType>::Sync(const Sync& sync)
  try
      : m_value((sync.m_mutex.lock(), sync.m_value)) {
    sync.m_mutex.unlock();
  } catch(...) {
    sync.m_mutex.unlock();
  }

  template<typename T, typename MutexType>
  Sync<T, MutexType>::Sync(Sync& sync)
      : Sync{static_cast<const Sync&>(sync)} {}

  template<typename T, typename MutexType>
  Sync<T, MutexType>::Sync(Sync&& sync)
  try
      : m_value((sync.m_mutex.lock(), std::move(sync.m_value))) {
    sync.m_mutex.unlock();
  } catch(...) {
    sync.m_mutex.unlock();
  }

  template<typename T, typename MutexType>
  T& Sync<T, MutexType>::AcquireReference() {
    boost::lock_guard<Mutex> lock(m_mutex);
    return m_value;
  }

  template<typename T, typename MutexType>
  T Sync<T, MutexType>::Acquire() const {
    boost::lock_guard<Mutex> lock(m_mutex);
    return m_value;
  }

  template<typename T, typename MutexType>
  boost::unique_lock<typename Sync<T, MutexType>::Mutex>& Sync<T, MutexType>::
      GetLock() const {
    return *m_lock;
  }

  template<typename T, typename MutexType>
  template<typename U>
  Sync<T, MutexType>& Sync<T, MutexType>::operator =(const Sync<U>& sync) {
    if(this == &sync) {
      return *this;
    }
    boost::lock_guard<Mutex> lock(m_mutex);
    sync.With(
      [&] (const U& value) {
        m_value = value;
      });
    return *this;
  }

  template<typename T, typename MutexType>
  template<typename U>
  Sync<T, MutexType>& Sync<T, MutexType>::operator =(Sync<U>&& sync) {
    if(this == &sync) {
      return *this;
    }
    boost::lock_guard<Mutex> lock(m_mutex);
    sync.With(
      [&] (U& value) {
        m_value = std::move(value);
      });
    return *this;
  }

  template<typename T, typename MutexType>
  template<typename ValueForward>
  Sync<T, MutexType>& Sync<T, MutexType>::operator =(ValueForward&& value) {
    boost::lock_guard<Mutex> lock(m_mutex);
    m_value = std::forward<ValueForward>(value);
    return *this;
  }
}
}

#endif
