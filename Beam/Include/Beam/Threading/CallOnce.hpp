#ifndef BEAM_CALL_ONCE_HPP
#define BEAM_CALL_ONCE_HPP
#include <atomic>
#include <boost/thread/locks.hpp>
#include "Beam/Threading/Threading.hpp"

namespace Beam::Threading {

  /**
   * Calls a function only once, blocking all other threads until the function
   * completes.
   * @param <M> The type of mutex to use to block other threads waiting for the
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
      CallOnce();

      /**
       * Invokes a function if no previous function invocation has taken place.
       * @param f The function to call.
       * @return <code>true</code> iff the function was called.
       */
      template<typename F>
      bool Call(F&& f);

    private:
      std::atomic_bool m_isInitialized;
      Mutex m_mutex;

      CallOnce(const CallOnce&) = delete;
      CallOnce& operator =(const CallOnce&) = delete;
  };

  template<typename M>
  CallOnce<M>::CallOnce()
    : m_isInitialized(false) {}

  template<typename M>
  template<typename F>
  bool CallOnce<M>::Call(F&& f) {
    if(m_isInitialized) {
      return false;
    }
    auto lock = boost::lock_guard(m_mutex);
    if(m_isInitialized) {
      return false;
    }
    f();
    m_isInitialized = true;
    return true;
  }
}

#endif
