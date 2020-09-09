#ifndef BEAM_SYNC_HPP
#define BEAM_SYNC_HPP
#include <type_traits>
#include <variant>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/utility/declval.hpp>
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam::Threading {
namespace Details {
  template<typename T, typename = void>
  struct IsSharedMutex : std::false_type {};

  template<typename T>
  struct IsSharedMutex<T, std::enable_if_t<
      std::is_void_v<decltype(boost::declval<T>().lock_shared())> &&
      std::is_same_v<decltype(boost::declval<T>().try_lock_shared()), bool> &&
      std::is_void_v<decltype(boost::declval<T>().unlock_shared())>>> :
    std::true_type {};

  template<typename T, typename = void>
  struct ReadLock {
    using type = boost::unique_lock<T>;
  };

  template<typename T>
  struct ReadLock<T, std::enable_if_t<IsSharedMutex<T>::value>> {
    using type = boost::shared_lock<T>;
  };

  template<typename T>
  using GetReadLock = typename ReadLock<T>::type;

  template<typename T>
  struct WriteLock {
    using type = boost::unique_lock<T>;
  };

  template<typename T>
  using GetWriteLock = typename WriteLock<T>::type;
}

  /*! \class Sync
      \brief Synchronizes access to a resource.
      \tparam T The type of value to store.
      \tparam M The type of mutex to use.
   */
  BEAM_SUPPRESS_MULTIPLE_CONSTRUCTORS()
  template<typename T, typename M = boost::mutex>
  class Sync {
    public:

      //! The value type stored.
      using Value = T;

      //! The type of mutex used.
      using Mutex = M;

      //! The type of the lock used for immutable access.
      using ReadLock = Details::GetReadLock<Mutex>;

      //! The type of the lock used for mutable access.
      using WriteLock = Details::GetWriteLock<Mutex>;

      //! The proxy for any type of lock in use.
      class LockProxy {
        public:

          //! Locks the referenced lock.
          void lock();

          //! Tries to lock the referenced lock.
          bool try_lock();

          //! Unlocks the referenced lock.
          void unlock();

        private:
          template<typename, typename> friend class Sync;
          using Variant = std::conditional_t<std::is_same_v<ReadLock,
            WriteLock>, std::variant<WriteLock>, std::variant<ReadLock,
            WriteLock>>;
          Variant m_lock;

          LockProxy() = default;
          LockProxy(const LockProxy&) = delete;
          explicit LockProxy(Variant lock);
      };

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

      //! Returns a copy of the value.
      T Acquire() const;

      //! Acquire Sync's value in a synchronized manner.
      /*!
        \param f The action to perform on the value.
      */
      template<typename F>
      decltype(auto) With(F&& f);

      //! Acquires Sync's value in a synchronized manner.
      /*!
        \param f The action to perform on the value.
      */
      template<typename F>
      decltype(auto) With(F&& f) const;

      //! Atomically acquires Sync's value along with another Sync's value.
      /*!
        \param s2 The other Sync.
        \param f The action to perform on the values.
      */
      template<typename S2, typename M2, typename F>
      decltype(auto) With(Sync<S2, M2>& s2, F&& f);

      //! Returns the lock used for synchronization.
      LockProxy& GetLock() const;

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
        friend LockRelease<LockProxy> Release(Sync<S1, M1>&);
      mutable Mutex m_mutex;
      mutable LockProxy* m_lock;
      Value m_value;
  };
  BEAM_UNSUPPRESS_MULTIPLE_CONSTRUCTORS()

  //! Acquires a Sync's instance in a synchronized manner.
  /*!
    \param sync The Sync whose instance is to be acquired.
    \param f The action to perform on the instance.
  */
  template<typename S1, typename M1, typename F>
  decltype(auto) With(Sync<S1, M1>& s1, F&& f) {
    return s1.With(std::forward<F>(f));
  }

  //! Acquires a Sync's instance in a synchronized manner.
  /*!
    \param sync The Sync whose instance is to be acquired.
    \param f The action to perform on the instance.
  */
  template<typename S1, typename M1, typename F>
  decltype(auto) With(const Sync<S1, M1>& s1, F&& f) {
    return s1.With(std::forward<F>(f));
  }

  //! Acquires two Syncs' instances in a synchronized manner.
  /*!
    \param s1 The first Sync whose instance is to be acquired.
    \param s2 The second Sync whose instance is to be acquired.
    \param f The action to perform on the instances.
  */
  template<typename S1, typename M1, typename S2, typename M2, typename F>
  decltype(auto) With(Sync<S1, M1>& s1, Sync<S2, M2>& s2, F&& f) {
    return s1.With(s2, std::forward<F>(f));
  }

  //! Releases a Sync.
  template<typename S1, typename M1>
  LockRelease<typename Sync<S1, M1>::LockProxy> Release(Sync<S1, M1>& s1) {
    return LockRelease(*s1.m_lock);
  }

  template<typename T, typename M>
  void Sync<T, M>::LockProxy::lock() {
    std::visit(
      [] (auto& lock) {
        lock.lock();
      }, m_lock);
  }

  template<typename T, typename M>
  bool Sync<T, M>::LockProxy::try_lock() {
    return std::visit(
      [] (auto& lock) {
        return lock.try_lock();
      }, m_lock);
  }

  template<typename T, typename M>
  void Sync<T, M>::LockProxy::unlock() {
    std::visit(
      [] (auto& lock) {
        lock.unlock();
      }, m_lock);
  }

  template<typename T, typename M>
  Sync<T, M>::LockProxy::LockProxy(Variant lock)
    : m_lock(std::move(lock)) {}

  template<typename T, typename M>
  template<typename... Args>
  Sync<T, M>::Sync(Args&&... args)
    : m_value(std::forward<Args>(args)...) {}

  template<typename T, typename M>
  Sync<T, M>::Sync(const Sync& sync)
  try
      : m_value((sync.m_mutex.lock(), sync.m_value)) {
    sync.m_mutex.unlock();
  } catch(...) {
    sync.m_mutex.unlock();
  }

  template<typename T, typename M>
  Sync<T, M>::Sync(Sync& sync)
    : Sync(static_cast<const Sync&>(sync)) {}

  template<typename T, typename M>
  Sync<T, M>::Sync(Sync&& sync)
  try
      : m_value((sync.m_mutex.lock(), std::move(sync.m_value))) {
    sync.m_mutex.unlock();
  } catch(...) {
    sync.m_mutex.unlock();
  }

  template<typename T, typename M>
  T Sync<T, M>::Acquire() const {
    auto lock = ReadLock(m_mutex);
    return m_value;
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) Sync<T, M>::With(F&& f) {
    auto lock = LockProxy(WriteLock(m_mutex));
    m_lock = &lock;
    return f(m_value);
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) Sync<T, M>::With(F&& f) const {
    auto lock = LockProxy(ReadLock(m_mutex));
    m_lock = &lock;
    return f(m_value);
  }

  template<typename T, typename M>
  template<typename S2, typename M2, typename F>
  decltype(auto) Sync<T, M>::With(Sync<S2, M2>& s2, F&& f) {
    auto lock1 = LockProxy(WriteLock(m_mutex, boost::defer_lock));
    auto lock2 = typename Sync<S2, M2>::LockProxy(
      typename Sync<S2, M2>::WriteLock(s2.m_mutex, boost::defer_lock));
    boost::lock(lock1, lock2);
    m_lock = &lock1;
    s2.m_lock = &lock2;
    return f(m_value, s2.m_value);
  }

  template<typename T, typename M>
  typename Sync<T, M>::LockProxy& Sync<T, M>::GetLock() const {
    return *m_lock;
  }

  template<typename T, typename M>
  template<typename U>
  Sync<T, M>& Sync<T, M>::operator =(const Sync<U>& sync) {
    if(this == &sync) {
      return *this;
    }
    auto lock = WriteLock(m_mutex);
    sync.With(
      [&] (const U& value) {
        m_value = value;
      });
    return *this;
  }

  template<typename T, typename M>
  template<typename U>
  Sync<T, M>& Sync<T, M>::operator =(Sync<U>&& sync) {
    if(this == &sync) {
      return *this;
    }
    auto lock = WriteLock(m_mutex);
    sync.With(
      [&] (U& value) {
        m_value = std::move(value);
      });
    return *this;
  }

  template<typename T, typename M>
  template<typename ValueForward>
  Sync<T, M>& Sync<T, M>::operator =(ValueForward&& value) {
    auto lock = WriteLock(m_mutex);
    m_value = std::forward<ValueForward>(value);
    return *this;
  }
}

#endif
