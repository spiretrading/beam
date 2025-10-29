#ifndef BEAM_CALL_ONCE_HPP
#define BEAM_CALL_ONCE_HPP
#include <atomic>
#include <concepts>
#include <boost/thread/locks.hpp>

namespace Beam {

  /**
   * Calls a function only once, blocking all other threads until the function
   * completes.
   * @tparam M The type of mutex to use to block other threads waiting for the
   *        function call to complete.
   */
  template<typename M>
  class CallOnce {
    public:

      /**
       * The type of mutex to use to block other threads waiting for the
       * function call to complete.
       */
      using Mutex = M;

      /** Constructs a CallOnce. */
      CallOnce() noexcept;

      /**
       * Invokes a function if no previous function invocation has taken place.
       * @param f The function to call.
       * @return <code>true</code> iff the function was called.
       */
      template<std::invocable<> F>
      bool call(F&& f);

    private:
      std::atomic_bool m_is_initialized;
      Mutex m_mutex;

      CallOnce(const CallOnce&) = delete;
      CallOnce& operator =(const CallOnce&) = delete;
  };

  template<typename M>
  CallOnce<M>::CallOnce() noexcept
    : m_is_initialized(false) {}

  template<typename M>
  template<std::invocable<> F>
  bool CallOnce<M>::call(F&& f) {
    if(m_is_initialized.load(std::memory_order_acquire)) {
      return false;
    }
    auto lock = boost::lock_guard(m_mutex);
    if(m_is_initialized.load(std::memory_order_acquire)) {
      return false;
    }
    std::forward<F>(f)();
    m_is_initialized.store(true, std::memory_order_release);
    return true;
  }
}

#endif
