#ifndef BEAM_CALL_ONCE_HPP
#define BEAM_CALL_ONCE_HPP
#include <atomic>
#include <boost/thread/locks.hpp>
#include "Beam/Threading/Threading.hpp"

namespace Beam {
namespace Threading {

  /*! \class CallOnce
      \brief Calls a function only once, blocking all other threads until the
             function completes.
      \tparam MutexType The type of mutex to use to block other threads waiting
              for the function call to complete.
   */
  template<typename MutexType>
  class CallOnce : private boost::noncopyable {
    public:

      //! The type of mutex to use to block other threads waiting for the
      //! function call to complete.
      using Mutex = MutexType;

      //! Constructs a CallOnce.
      CallOnce();

      //! Invokes a function if no previous function invocation has taken place.
      /*!
        \param f The function to call.
        \return <code>true</code> iff the function was called.
      */
      template<typename F>
      bool Call(const F& f);

    private:
      std::atomic_bool m_isInitialized;
      Mutex m_mutex;
  };

  template<typename MutexType>
  CallOnce<MutexType>::CallOnce()
      : m_isInitialized(false) {}

  template<typename MutexType>
  template<typename F>
  bool CallOnce<MutexType>::Call(const F& f) {
    if(m_isInitialized) {
      return false;
    }
    boost::lock_guard<Mutex> lock(m_mutex);
    if(m_isInitialized) {
      return false;
    }
    f();
    m_isInitialized = true;
    return true;
  }
}
}

#endif
