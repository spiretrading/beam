#ifndef BEAM_REMOTE_HPP
#define BEAM_REMOTE_HPP
#include <functional>
#include <boost/noncopyable.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Pointers/DelayPtr.hpp"
#include "Beam/Threading/PreferredConditionVariable.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class Remote
      \brief Represents a value that must be loaded remotely using blocking
             calls.
   */
  template<typename T, typename MutexType = boost::mutex>
  class Remote : private boost::noncopyable {
    public:

      //! Defines the function used to initialize the Remote value.
      /*!
        \param value The value to initialize.
      */
      typedef std::function<void (DelayPtr<T>& value)> InitializationFunction;

      //! Constructs a Remote object.
      Remote();

      //! Constructs a Remote object.
      /*!
        \param initializer The function used to initialize this object.
      */
      Remote(const InitializationFunction& initializer);

      //! Sets the function used to initialize the Remote value.
      /*!
        \param initialize The function used to initialize this object.
      */
      void SetInitializationFunction(const InitializationFunction& initializer);

      //! Returns <code>true</code> iff the object was loaded and is available.
      bool IsAvailable() const;

      //! Returns the Remote value, initializing it if needed.
      T& operator *() const;

      //! Returns the Remote value, initializing it if needed.
      T* operator ->() const;

    private:
      mutable MutexType m_mutex;
      InitializationFunction m_initialize;
      mutable DelayPtr<T> m_value;
      mutable bool m_isAvailable;
      mutable bool m_isLoading;
      mutable typename Threading::PreferredConditionVariable<MutexType>::type
        m_isAvailableCondition;
  };

  template<typename T, typename MutexType>
  Remote<T, MutexType>::Remote()
      : m_isAvailable(false),
        m_isLoading(false) {}

  template<typename T, typename MutexType>
  Remote<T, MutexType>::Remote(const InitializationFunction& initializer)
      : m_isAvailable(false),
        m_isLoading(false),
        m_initialize(initializer) {}

  template<typename T, typename MutexType>
  void Remote<T, MutexType>::SetInitializationFunction(
      const InitializationFunction& initializer) {
    m_initialize = initializer;
  }

  template<typename T, typename MutexType>
  bool Remote<T, MutexType>::IsAvailable() const {
    boost::lock_guard<MutexType> lock(m_mutex);
    return m_isAvailable;
  }

  template<typename T, typename MutexType>
  T& Remote<T, MutexType>::operator *() const {
    boost::unique_lock<MutexType> lock(m_mutex);
    if(!m_isAvailable) {
      if(!m_isLoading) {
        m_isLoading = true;
        {
          auto release = Threading::Release(lock);
          m_initialize(m_value);
        }
        m_isLoading = false;
        m_isAvailable = true;
        m_isAvailableCondition.notify_all();
      }
    }
    while(!m_isAvailable) {
      m_isAvailableCondition.wait(lock);
    }
    return *m_value;
  }

  template<typename T, typename MutexType>
  T* Remote<T, MutexType>::operator ->() const {
    return &**this;
  }
}

#endif
