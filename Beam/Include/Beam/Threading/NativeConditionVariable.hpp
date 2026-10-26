#ifndef BEAM_NATIVE_CONDITION_VARIABLE_HPP
#define BEAM_NATIVE_CONDITION_VARIABLE_HPP
#include <chrono>
#include <condition_variable>
#include <mutex>
#include "Beam/Threading/NativeMutex.hpp"

namespace Beam {

  /**
   * A condition variable that blocks the calling OS thread while waiting.
   * Bypasses Beam's synchronization hooks, including when used by a routine.
   */
  class NativeConditionVariable {
    public:

      /** Constructs a condition variable. */
      NativeConditionVariable();

      ~NativeConditionVariable();

      /**
       * Releases a mutex and waits, reacquiring the mutex before returning.
       * May wake spuriously.
       * @param lock A lock owned by the calling OS thread. Concurrent waiters
       *        must use the same mutex.
       */
      void wait(std::unique_lock<NativeMutex>& lock);

      /**
       * Waits until a predicate is true, testing it with the mutex locked.
       * @param lock A lock owned by the calling OS thread. Concurrent waiters
       *        must use the same mutex.
       * @param predicate Returns whether waiting is complete.
       */
      void wait(std::unique_lock<NativeMutex>& lock, auto predicate);

      /**
       * Waits for a notification or a duration measured by a steady clock.
       * @param lock A lock satisfying the same requirements as wait().
       * @param timeout The maximum duration to wait.
       * @return Whether the timeout expired. May wake spuriously.
       */
      template<typename Rep, typename Period>
      std::cv_status wait_for(std::unique_lock<NativeMutex>& lock,
        std::chrono::duration<Rep, Period> timeout);

      /**
       * Waits for a predicate or a duration measured by a steady clock.
       * @param lock A lock satisfying the same requirements as wait().
       * @param timeout The maximum duration to wait.
       * @param predicate Returns whether waiting is complete.
       * @return The predicate's value with the mutex locked.
       */
      template<typename Rep, typename Period>
      bool wait_for(std::unique_lock<NativeMutex>& lock,
        std::chrono::duration<Rep, Period> timeout, auto predicate);

      /**
       * Waits for a notification or an absolute deadline.
       * @param lock A lock satisfying the same requirements as wait().
       * @param deadline The time at which waiting expires.
       * @return Whether the deadline expired. May wake spuriously.
       */
      template<typename Clock, typename Duration>
      std::cv_status wait_until(std::unique_lock<NativeMutex>& lock,
        std::chrono::time_point<Clock, Duration> deadline);

      /**
       * Waits for a predicate or an absolute deadline.
       * @param lock A lock satisfying the same requirements as wait().
       * @param deadline The time at which waiting expires.
       * @param predicate Returns whether waiting is complete.
       * @return The predicate's value with the mutex locked.
       */
      template<typename Clock, typename Duration>
      bool wait_until(std::unique_lock<NativeMutex>& lock,
        std::chrono::time_point<Clock, Duration> deadline, auto predicate);

      /** Wakes one waiting thread, if any. */
      void notify_one();

      /** Wakes all waiting threads. */
      void notify_all();

    private:
      struct Functions {
        #ifdef _WIN32
          decltype(&SleepConditionVariableSRW) m_wait =
            NativeMutex::load<decltype(m_wait)>("SleepConditionVariableSRW");
          decltype(&WakeConditionVariable) m_notify_one =
            NativeMutex::load<decltype(m_notify_one)>("WakeConditionVariable");
          decltype(&WakeAllConditionVariable) m_notify_all =
            NativeMutex::load<decltype(m_notify_all)>(
              "WakeAllConditionVariable");
        #elif defined(__linux__) && defined(__GLIBC__) && defined(__x86_64__)
          decltype(&pthread_cond_wait) m_wait =
            NativeMutex::load<decltype(m_wait)>("pthread_cond_wait");
          decltype(&pthread_cond_clockwait) m_timed_wait =
            NativeMutex::load<decltype(m_timed_wait)>("pthread_cond_clockwait");
          decltype(&pthread_cond_signal) m_notify_one =
            NativeMutex::load<decltype(m_notify_one)>("pthread_cond_signal");
          decltype(&pthread_cond_broadcast) m_notify_all =
            NativeMutex::load<decltype(m_notify_all)>("pthread_cond_broadcast");
        #else
          decltype(&pthread_cond_wait) m_wait = &pthread_cond_wait;
          decltype(&pthread_cond_timedwait) m_timed_wait =
            &pthread_cond_timedwait;
          decltype(&pthread_cond_signal) m_notify_one = &pthread_cond_signal;
          decltype(&pthread_cond_broadcast) m_notify_all =
            &pthread_cond_broadcast;
        #endif
      };
      #ifdef _WIN32
        CONDITION_VARIABLE m_condition;
      #else
        pthread_cond_t m_condition;
      #endif

      static const Functions& get_functions();
      std::cv_status wait_native(std::unique_lock<NativeMutex>& lock,
        std::chrono::nanoseconds timeout, bool is_steady);
      NativeConditionVariable(const NativeConditionVariable&) = delete;
      NativeConditionVariable& operator =(
        const NativeConditionVariable&) = delete;
  };

  inline NativeConditionVariable::NativeConditionVariable()
      : m_condition() {
    get_functions();
    #ifndef _WIN32
      auto result = pthread_cond_init(&m_condition, nullptr);
      if(result != 0) {
        throw std::system_error(result, std::generic_category());
      }
    #endif
  }

  inline NativeConditionVariable::~NativeConditionVariable() {
    #ifndef _WIN32
      pthread_cond_destroy(&m_condition);
    #endif
  }

  inline void NativeConditionVariable::wait(
      std::unique_lock<NativeMutex>& lock) {
    #ifdef _WIN32
      if(!get_functions().m_wait(
          &m_condition, &lock.mutex()->m_mutex, INFINITE, 0)) {
        throw std::system_error(GetLastError(), std::system_category());
      }
    #else
      auto result =
        get_functions().m_wait(&m_condition, &lock.mutex()->m_mutex);
      if(result != 0) {
        throw std::system_error(result, std::generic_category());
      }
    #endif
  }

  inline void NativeConditionVariable::wait(
      std::unique_lock<NativeMutex>& lock, auto predicate) {
    while(!predicate()) {
      wait(lock);
    }
  }

  template<typename Rep, typename Period>
  std::cv_status NativeConditionVariable::wait_for(
      std::unique_lock<NativeMutex>& lock,
      std::chrono::duration<Rep, Period> timeout) {
    using Seconds = std::chrono::duration<long double>;
    auto deadline = std::chrono::time_point<std::chrono::steady_clock, Seconds>(
      Seconds(std::chrono::steady_clock::now().time_since_epoch()) +
        Seconds(timeout));
    return wait_until(lock, deadline);
  }

  template<typename Rep, typename Period>
  bool NativeConditionVariable::wait_for(std::unique_lock<NativeMutex>& lock,
      std::chrono::duration<Rep, Period> timeout, auto predicate) {
    using Seconds = std::chrono::duration<long double>;
    auto deadline = std::chrono::time_point<std::chrono::steady_clock, Seconds>(
      Seconds(std::chrono::steady_clock::now().time_since_epoch()) +
        Seconds(timeout));
    return wait_until(lock, deadline, std::move(predicate));
  }

  template<typename Clock, typename Duration>
  std::cv_status NativeConditionVariable::wait_until(
      std::unique_lock<NativeMutex>& lock,
      std::chrono::time_point<Clock, Duration> deadline) {
    using Seconds = std::chrono::duration<long double>;
    while(true) {
      auto remaining = Seconds(deadline.time_since_epoch()) -
        Seconds(Clock::now().time_since_epoch());
      auto timeout = std::chrono::nanoseconds::zero();
      if(remaining > Seconds::zero()) {
        if(remaining > std::chrono::hours(24)) {
          timeout = std::chrono::hours(24);
        } else {
          timeout = std::chrono::ceil<std::chrono::nanoseconds>(remaining);
        }
      }
      auto status = wait_native(lock, timeout, Clock::is_steady);
      if(Seconds(Clock::now().time_since_epoch()) >=
          Seconds(deadline.time_since_epoch())) {
        return std::cv_status::timeout;
      }
      if(status == std::cv_status::no_timeout) {
        return status;
      }
    }
  }

  template<typename Clock, typename Duration>
  bool NativeConditionVariable::wait_until(std::unique_lock<NativeMutex>& lock,
      std::chrono::time_point<Clock, Duration> deadline, auto predicate) {
    while(!predicate()) {
      if(wait_until(lock, deadline) == std::cv_status::timeout) {
        return predicate();
      }
    }
    return true;
  }

  inline void NativeConditionVariable::notify_one() {
    #ifdef _WIN32
      get_functions().m_notify_one(&m_condition);
    #else
      auto result = get_functions().m_notify_one(&m_condition);
      if(result != 0) {
        throw std::system_error(result, std::generic_category());
      }
    #endif
  }

  inline void NativeConditionVariable::notify_all() {
    #ifdef _WIN32
      get_functions().m_notify_all(&m_condition);
    #else
      auto result = get_functions().m_notify_all(&m_condition);
      if(result != 0) {
        throw std::system_error(result, std::generic_category());
      }
    #endif
  }

  inline const NativeConditionVariable::Functions&
      NativeConditionVariable::get_functions() {
    static const auto functions = Functions();
    return functions;
  }

  inline std::cv_status NativeConditionVariable::wait_native(
      std::unique_lock<NativeMutex>& lock, std::chrono::nanoseconds timeout,
      bool is_steady) {
    #ifdef _WIN32
      static_cast<void>(is_steady);
      auto milliseconds = std::chrono::ceil<std::chrono::milliseconds>(timeout);
      if(get_functions().m_wait(&m_condition, &lock.mutex()->m_mutex,
          static_cast<DWORD>(milliseconds.count()), 0)) {
        return std::cv_status::no_timeout;
      }
      auto error = GetLastError();
      if(error != ERROR_TIMEOUT) {
        throw std::system_error(error, std::system_category());
      }
    #else
      auto clock = CLOCK_REALTIME;
      #if defined(__linux__) && defined(__GLIBC__) && defined(__x86_64__)
        if(is_steady) {
          clock = CLOCK_MONOTONIC;
        }
      #else
        static_cast<void>(is_steady);
      #endif
      auto deadline = timespec();
      if(clock_gettime(clock, &deadline) != 0) {
        throw std::system_error(errno, std::generic_category());
      }
      auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeout);
      deadline.tv_sec += seconds.count();
      deadline.tv_nsec += (timeout - seconds).count();
      if(deadline.tv_nsec >= 1000000000) {
        ++deadline.tv_sec;
        deadline.tv_nsec -= 1000000000;
      }
      #if defined(__linux__) && defined(__GLIBC__) && defined(__x86_64__)
        auto result = get_functions().m_timed_wait(
          &m_condition, &lock.mutex()->m_mutex, clock, &deadline);
      #else
        auto result = get_functions().m_timed_wait(
          &m_condition, &lock.mutex()->m_mutex, &deadline);
      #endif
      if(result == 0) {
        return std::cv_status::no_timeout;
      }
      if(result != ETIMEDOUT) {
        throw std::system_error(result, std::generic_category());
      }
    #endif
    return std::cv_status::timeout;
  }
}

#endif
