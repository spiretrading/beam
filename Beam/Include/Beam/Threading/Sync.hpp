#ifndef BEAM_SYNC_HPP
#define BEAM_SYNC_HPP
#include <shared_mutex>
#include <type_traits>
#include <variant>
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {
namespace Details {
  template<typename T>
  concept IsSharedMutex = requires(T t) {
    { t.lock_shared() } -> std::same_as<void>;
    { t.try_lock_shared() } -> std::same_as<bool>;
    { t.unlock_shared() } -> std::same_as<void>;
  };

  template<typename T>
  struct read_lock {
    using type = std::unique_lock<T>;
  };

  template<IsSharedMutex T>
  struct read_lock<T> {
    using type = std::shared_lock<T>;
  };

  template<typename T>
  using read_lock_t = typename read_lock<T>::type;

  template<typename T>
  struct write_lock {
    using type = std::unique_lock<T>;
  };

  template<typename T>
  using write_lock_t = typename write_lock<T>::type;
}

  /**
   * Synchronizes access to a resource.
   * @tparam T The type of value to store.
   * @tparam M The type of mutex to use.
   */
  template<typename T, typename M = boost::mutex>
  class Sync {
    public:

      /** The value type stored. */
      using Value = T;

      /** The type of mutex used. */
      using Mutex = M;

      /** The type of the lock used for immutable access. */
      using ReadLock = Details::read_lock_t<Mutex>;

      /** The type of the lock used for mutable access. */
      using WriteLock = Details::write_lock_t<Mutex>;

      /** The proxy for any type of lock in use. */
      class LockProxy {
        public:

          /** Locks the referenced lock. */
          void lock();

          /** Tries to lock the referenced lock. */
          bool try_lock();

          /** Unlocks the referenced lock. */
          void unlock();

        private:
          template<typename, typename> friend class Sync;
          using Variant = std::conditional_t<
            std::is_same_v<ReadLock, WriteLock>, std::variant<WriteLock>,
            std::variant<ReadLock, WriteLock>>;
          Variant m_lock;

          LockProxy() = default;
          LockProxy(const LockProxy&) = delete;
          explicit LockProxy(Variant lock);
      };

      /**
       * Constructs a Sync.
       * @param args The parameters to pass to the synchronized value.
       */
      template<
        typename... Args, typename = disable_copy_constructor_t<Sync, Args...>>
      Sync(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<Value, Args&&...>);

      /**
       * Constructs a Sync by converting or copying an existing Sync.
       * @param sync The sync to construct from.
       */
      template<typename U>
      Sync(const Sync<U>& sync) noexcept(
        std::is_nothrow_constructible_v<Value, const U&>);

      /**
       * Constructs a Sync by moving an existing Sync.
       * @param sync The sync to construct from.
       */
      template<typename U>
      Sync(Sync<U>&& sync) noexcept(
        std::is_nothrow_constructible_v<Value, U&&>);

      Sync(const Sync& sync) noexcept(
        std::is_nothrow_copy_constructible_v<Value>);

      /** Returns a copy of the value. */
      T load() const;

      /**
       * Acquire Sync's value in a synchronized manner.
       * @param f The action to perform on the value.
       */
      template<typename F>
      decltype(auto) with(F&& f);

      /**
       * Acquires Sync's value in a synchronized manner.
       * @param f The action to perform on the value.
       */
      template<typename F>
      decltype(auto) with(F&& f) const;

      /**
       * Atomically acquires Sync's value along with another Sync's value.
       * @param s2 The other Sync.
       * @param f The action to perform on the values.
       */
      template<typename S2, typename M2, typename F>
      decltype(auto) with(Sync<S2, M2>& s2, F&& f);

      /** Returns the lock used for synchronization. */
      LockProxy& get_lock() const;

      template<typename U, typename = disable_copy_constructor_t<Sync, U>>
      Sync& operator =(U&& value) noexcept(
        std::is_nothrow_assignable_v<Value, U&&>);
      template<typename U>
      Sync& operator =(const Sync<U>& sync) noexcept(
        std::is_nothrow_assignable_v<Value, const U&>);
      template<typename U>
      Sync& operator =(Sync<U>&& sync) noexcept(
        std::is_nothrow_assignable_v<Value, U&&>);
      Sync& operator =(const Sync& sync) noexcept(
        std::is_nothrow_copy_assignable_v<Value>);

    private:
      template<typename, typename> friend class Sync;
      template<typename S1, typename M1>
      friend LockRelease<LockProxy> release(Sync<S1, M1>&);
      mutable Mutex m_mutex;
      mutable LockProxy* m_lock;
      Value m_value;
  };

  template<typename T>
  Sync(T) -> Sync<std::remove_cvref_t<T>>;

  template<typename U, typename M>
  Sync(const Sync<U, M>&) -> Sync<U, M>;

  template<typename U, typename M>
  Sync(Sync<U, M>&&) -> Sync<U, M>;

  /**
   * Acquires a Sync's instance in a synchronized manner.
   * @param sync The Sync whose instance is to be acquired.
   * @param f The action to perform on the instance.
   */
  template<typename S1, typename M1, typename F>
  decltype(auto) with(Sync<S1, M1>& s1, F&& f) {
    return s1.with(std::forward<F>(f));
  }

  /**
   * Acquires a Sync's instance in a synchronized manner.
   * @param sync The Sync whose instance is to be acquired.
   * @param f The action to perform on the instance.
   */
  template<typename S1, typename M1, typename F>
  decltype(auto) with(const Sync<S1, M1>& s1, F&& f) {
    return s1.with(std::forward<F>(f));
  }

  /**
   * Acquires two Syncs' instances in a synchronized manner.
   * @param s1 The first Sync whose instance is to be acquired.
   * @param s2 The second Sync whose instance is to be acquired.
   * @param f The action to perform on the instances.
   */
  template<typename S1, typename M1, typename S2, typename M2, typename F>
  decltype(auto) with(Sync<S1, M1>& s1, Sync<S2, M2>& s2, F&& f) {
    return s1.with(s2, std::forward<F>(f));
  }

  /** Releases a Sync. */
  template<typename S1, typename M1>
  auto release(Sync<S1, M1>& s1) {
    return LockRelease(*s1.m_lock);
  }

  template<typename T, typename M>
  void Sync<T, M>::LockProxy::lock() {
    std::visit([] (auto& lock) {
      lock.lock();
    }, m_lock);
  }

  template<typename T, typename M>
  bool Sync<T, M>::LockProxy::try_lock() {
    return std::visit([] (auto& lock) {
      return lock.try_lock();
    }, m_lock);
  }

  template<typename T, typename M>
  void Sync<T, M>::LockProxy::unlock() {
    std::visit([] (auto& lock) {
      lock.unlock();
    }, m_lock);
  }

  template<typename T, typename M>
  Sync<T, M>::LockProxy::LockProxy(Variant lock)
    : m_lock(std::move(lock)) {}

  template<typename T, typename M>
  template<typename... Args, typename>
  Sync<T, M>::Sync(Args&&... args) noexcept(
    std::is_nothrow_constructible_v<Value, Args&&...>)
    : m_value(std::forward<Args>(args)...) {}

  template<typename T, typename M>
  template<typename U>
  Sync<T, M>::Sync(const Sync<U>& sync) noexcept(
    std::is_nothrow_constructible_v<Value, const U&>)
  try
      : m_value((sync.m_mutex.lock(), sync.m_value)) {
    sync.m_mutex.unlock();
  } catch(...) {
    sync.m_mutex.unlock();
  }

  template<typename T, typename M>
  template<typename U>
  Sync<T, M>::Sync(Sync<U>&& sync) noexcept(
    std::is_nothrow_constructible_v<Value, U&&>)
  try
      : m_value((sync.m_mutex.lock(), std::move(sync.m_value))) {
    sync.m_mutex.unlock();
  } catch(...) {
    sync.m_mutex.unlock();
  }

  template<typename T, typename M>
  Sync<T, M>::Sync(const Sync& sync) noexcept(
    std::is_nothrow_copy_constructible_v<Value>)
  try
      : m_value((sync.m_mutex.lock(), sync.m_value)) {
    sync.m_mutex.unlock();
  } catch(...) {
    sync.m_mutex.unlock();
  }

  template<typename T, typename M>
  typename Sync<T, M>::Value Sync<T, M>::load() const {
    auto lock = ReadLock(m_mutex);
    return m_value;
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) Sync<T, M>::with(F&& f) {
    auto lock = LockProxy(WriteLock(m_mutex));
    m_lock = &lock;
    return f(m_value);
  }

  template<typename T, typename M>
  template<typename F>
  decltype(auto) Sync<T, M>::with(F&& f) const {
    auto lock = LockProxy(ReadLock(m_mutex));
    m_lock = &lock;
    return f(m_value);
  }

  template<typename T, typename M>
  template<typename S2, typename M2, typename F>
  decltype(auto) Sync<T, M>::with(Sync<S2, M2>& s2, F&& f) {
    auto lock1 = LockProxy(WriteLock(m_mutex, std::defer_lock));
    auto lock2 = typename Sync<S2, M2>::LockProxy(
      typename Sync<S2, M2>::WriteLock(s2.m_mutex, std::defer_lock));
    std::lock(lock1, lock2);
    m_lock = &lock1;
    s2.m_lock = &lock2;
    return f(m_value, s2.m_value);
  }

  template<typename T, typename M>
  typename Sync<T, M>::LockProxy& Sync<T, M>::get_lock() const {
    return *m_lock;
  }

  template<typename T, typename M>
  template<typename U, typename>
  Sync<T, M>& Sync<T, M>::operator =(U&& value) noexcept(
      std::is_nothrow_assignable_v<Value, U&&>) {
    auto lock = WriteLock(m_mutex);
    m_value = std::forward<U>(value);
    return *this;
  }

  template<typename T, typename M>
  template<typename U>
  Sync<T, M>& Sync<T, M>::operator =(const Sync<U>& sync) noexcept(
      std::is_nothrow_assignable_v<Value, const U&>) {
    if constexpr(std::is_same_v<T, U>) {
      if(this == &sync) {
        return *this;
      }
    }
    auto lock = WriteLock(m_mutex);
    sync.with([&] (const U& value) {
      m_value = value;
    });
    return *this;
  }

  template<typename T, typename M>
  template<typename U>
  Sync<T, M>& Sync<T, M>::operator =(Sync<U>&& sync) noexcept(
      std::is_nothrow_assignable_v<Value, U&&>) {
    if constexpr(std::is_same_v<T, U>) {
      if(this == &sync) {
        return *this;
      }
    }
    auto lock = WriteLock(m_mutex);
    sync.with([&] (U& value) {
      m_value = std::move(value);
    });
    return *this;
  }

  template<typename T, typename M>
  Sync<T, M>& Sync<T, M>::operator =(const Sync& sync) noexcept(
      std::is_nothrow_copy_assignable_v<Value>) {
    if(this == &sync) {
      return *this;
    }
    auto lock = WriteLock(m_mutex);
    sync.with([&] (const Value& value) {
      m_value = value;
    });
    return *this;
  }
}

#endif
