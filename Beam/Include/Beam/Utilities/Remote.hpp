#ifndef BEAM_REMOTE_HPP
#define BEAM_REMOTE_HPP
#include <atomic>
#include <exception>
#include <functional>
#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/LockRelease.hpp"
#include "Beam/Threading/PreferredConditionVariable.hpp"

namespace Beam {

  /**
   * Represents a value that must be loaded remotely using blocking calls.
   * @tparam T The type of value to load.
   * @tparam M The type of mutex used to synchronize the initialization.
   */
  template<typename T, typename M = boost::mutex>
  class Remote {
    public:

      /** The type of value to load. */
      using Type = T;

      /** The type of mutex used to synchronize the initialization. */
      using Mutex = M;

      /**
       * Defines the function used to initialize the Remote value.
       * @param value The value to initialize.
       */
      using Initializer = std::function<void (boost::optional<Type>& value)>;

      /** Constructs a Remote object. */
      Remote() noexcept;

      /**
       * Constructs a Remote object.
       * @param initializer The function used to initialize this object.
       */
      explicit Remote(Initializer initializer) noexcept;

      /**
       * Returns <code>true</code> iff the object was loaded and is available.
       */
      explicit operator bool() const;

      /**
       * Sets the function used to initialize the Remote value.
       * @param initializer The function used to initialize this object.
       */
      void set_initializer(const Initializer& initializer);

      /**
       * Returns <code>true</code> iff the object was loaded and is available.
       */
      bool is_available() const;

      /** Returns the Remote value, initializing it if needed. */
      Type& operator *() const;

      /** Returns the Remote value, initializing it if needed. */
      Type* operator ->() const;

    private:
      mutable Mutex m_mutex;
      Initializer m_initializer;
      mutable boost::optional<Type> m_value;
      mutable std::exception_ptr m_initialization_exception;
      mutable std::atomic_bool m_is_available;
      mutable bool m_is_loading;
      mutable preferred_condition_variable_t<Mutex> m_is_available_condition;

      Remote(const Remote&) = delete;
      Remote& operator =(const Remote&) = delete;
  };

  template<typename T, typename M>
  Remote<T, M>::Remote() noexcept
    : m_is_available(false),
      m_is_loading(false) {}

  template<typename T, typename M>
  Remote<T, M>::Remote(Initializer initializer) noexcept
    : m_is_available(false),
      m_is_loading(false),
      m_initializer(std::move(initializer)) {}

  template<typename T, typename M>
  Remote<T, M>::operator bool() const {
    return is_available();
  }

  template<typename T, typename M>
  void Remote<T, M>::set_initializer(const Initializer& initializer) {
    m_initializer = initializer;
  }

  template<typename T, typename M>
  bool Remote<T, M>::is_available() const {
    return m_is_available;
  }

  template<typename T, typename M>
  typename Remote<T, M>::Type& Remote<T, M>::operator *() const {
    if(m_is_available) {
      return *m_value;
    }
    auto lock = boost::unique_lock(m_mutex);
    if(!m_is_available && !m_is_loading) {
      m_is_loading = true;
      m_initialization_exception = nullptr;
      try {
        auto releaser = release(lock);
        m_initializer(m_value);
      } catch(...) {
        m_initialization_exception = std::current_exception();
        m_is_loading = false;
        m_is_available_condition.notify_all();
        throw;
      }
      m_is_loading = false;
      m_is_available = true;
      m_is_available_condition.notify_all();
    }
    while(!m_is_available && m_is_loading) {
      m_is_available_condition.wait(lock);
    }
    if(m_initialization_exception) {
      std::rethrow_exception(m_initialization_exception);
    }
    return *m_value;
  }

  template<typename T, typename M>
  typename Remote<T, M>::Type* Remote<T, M>::operator ->() const {
    return &**this;
  }
}

#endif
