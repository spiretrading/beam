#ifndef BEAM_REMOTE_HPP
#define BEAM_REMOTE_HPP
#include <atomic>
#include <functional>
#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Threading/PreferredConditionVariable.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /**
   * Represents a value that must be loaded remotely using blocking calls.
   * @param <T> The type of value to load.
   * @param <M> The type of mutex used to synchronize the initialization.
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
      using InitializationFunction =
        std::function<void (boost::optional<Type>& value)>;

      /** Constructs a Remote object. */
      Remote();

      /**
       * Constructs a Remote object.
       * @param initializer The function used to initialize this object.
       */
      explicit Remote(const InitializationFunction& initializer);

      /**
       * Sets the function used to initialize the Remote value.
       * @param initialize The function used to initialize this object.
       */
      void SetInitializationFunction(const InitializationFunction& initializer);

      /**
       * Returns <code>true</code> iff the object was loaded and is available.
       */
      bool IsAvailable() const;

      /** Returns the Remote value, initializing it if needed. */
      explicit Type& operator *() const;

      /** Returns the Remote value, initializing it if needed. */
      explicit Type* operator ->() const;

    private:
      mutable Mutex m_mutex;
      InitializationFunction m_initialize;
      mutable boost::optional<Type> m_value;
      mutable std::atomic_bool m_isAvailable;
      mutable bool m_isLoading;
      mutable typename Threading::PreferredConditionVariable<Mutex>::type
        m_isAvailableCondition;

      Remote(const Remote&) = delete;
      Remote& operator =(const Remote&) = delete;
  };

  template<typename T, typename M>
  Remote<T, M>::Remote()
    : m_isAvailable(false),
      m_isLoading(false) {}

  template<typename T, typename M>
  Remote<T, M>::Remote(const InitializationFunction& initializer)
    : m_isAvailable(false),
      m_isLoading(false),
      m_initialize(initializer) {}

  template<typename T, typename M>
  void Remote<T, M>::SetInitializationFunction(
      const InitializationFunction& initializer) {
    m_initialize = initializer;
  }

  template<typename T, typename M>
  bool Remote<T, M>::IsAvailable() const {
    return m_isAvailable;
  }

  template<typename T, typename M>
  typename Remote<T, M>::Type& Remote<T, M>::operator *() const {
    if(m_isAvailable) {
      return *m_value;
    }
    auto lock = boost::unique_lock(m_mutex);
    if(!m_isAvailable && !m_isLoading) {
      m_isLoading = true;
      {
        auto release = Threading::Release(lock);
        m_initialize(m_value);
      }
      m_isLoading = false;
      m_isAvailable = true;
      m_isAvailableCondition.notify_all();
    }
    while(!m_isAvailable) {
      m_isAvailableCondition.wait(lock);
    }
    return *m_value;
  }

  template<typename T, typename M>
  typename Remote<T, M>::Type* Remote<T, M>::operator ->() const {
    return &**this;
  }
}

#endif
