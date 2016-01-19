#ifndef BEAM_SCOPEDSLOTADAPTOR_HPP
#define BEAM_SCOPEDSLOTADAPTOR_HPP
#include <functional>
#include <utility>
#include <boost/noncopyable.hpp>
#include "Beam/SignalHandling/SignalHandling.hpp"
#include "Beam/Threading/RecursiveMutex.hpp"
#include "Beam/Threading/Sync.hpp"
#include "Beam/Utilities/Functional.hpp"

namespace Beam {
namespace SignalHandling {
namespace Details {
  template<typename F, typename T>
  struct Callback {
    std::shared_ptr<Threading::Sync<bool, Threading::RecursiveMutex>>
      m_keepAlive;
    F m_callback;
    T m_default;

    template<typename CallbackForward>
    Callback(std::shared_ptr<
        Threading::Sync<bool, Threading::RecursiveMutex>> keepAlive,
        CallbackForward&& callback, const T& defaultValue)
        : m_keepAlive(std::move(keepAlive)),
          m_callback(std::forward<CallbackForward>(callback)),
          m_default(defaultValue) {}

    template<typename... Args>
    T operator ()(Args&&... args) {
      return Threading::With(*m_keepAlive,
        [&] (bool keepAlive) {
          if(keepAlive) {
            return m_callback(std::forward<Args>(args)...);
          } else {
            return m_default;
          }
        });
    }
  };

  template<typename F>
  struct Callback<F, void> {
    std::shared_ptr<Threading::Sync<bool, Threading::RecursiveMutex>>
      m_keepAlive;
    F m_callback;

    template<typename CallbackForward>
    Callback(std::shared_ptr<
        Threading::Sync<bool, Threading::RecursiveMutex>> keepAlive,
        CallbackForward&& callback)
        : m_keepAlive(std::move(keepAlive)),
          m_callback(std::forward<CallbackForward>(callback)) {}

    template<typename... Args>
    void operator ()(Args&&... args) {
      return Threading::With(*m_keepAlive,
        [&] (bool keepAlive) {
          if(keepAlive) {
            m_callback(std::forward<Args>(args)...);
          }
        });
    }
  };
}

  /*! \class ScopedSlotAdaptor
      \brief Produces callbacks that only get invoked if the object that
             constructed it is in scope.
   */
  class ScopedSlotAdaptor : private boost::noncopyable {
    public:

      //! Constructs a ScopedSlotAdaptor.
      ScopedSlotAdaptor();

      ~ScopedSlotAdaptor();

      //! Returns a callback whose scope is tied to this object.
      /*!
        \param callback The callback to scope.
      */
      template<typename F>
      Details::Callback<typename std::decay<F>::type, void> GetCallback(
        F&& callback) const;

      //! Returns a callback whose scope is tied to this object.
      /*!
        \param callback The callback to scope.
        \param defaultValue The value to return if the callback is not in scope.
      */
      template<typename F, typename T>
      Details::Callback<typename std::decay<F>::type,
        typename std::decay<T>::type> GetCallback(F&& callback,
        T&& defaultValue) const;

    private:
      std::shared_ptr<Threading::Sync<bool, Threading::RecursiveMutex>>
        m_keepAlive;
  };

  inline ScopedSlotAdaptor::ScopedSlotAdaptor()
      : m_keepAlive(std::make_shared<
          Threading::Sync<bool, Threading::RecursiveMutex>>(true)) {}

  inline ScopedSlotAdaptor::~ScopedSlotAdaptor() {
    *m_keepAlive = false;
  }

  template<typename F>
  Details::Callback<typename std::decay<F>::type, void>
      ScopedSlotAdaptor::GetCallback(F&& callback) const {
    return Details::Callback<typename std::decay<F>::type, void>(m_keepAlive,
      std::forward<F>(callback));
  }

  template<typename F, typename T>
  Details::Callback<typename std::decay<F>::type, typename std::decay<T>::type>
      ScopedSlotAdaptor::GetCallback(F&& callback, T&& defaultValue) const {
    return Details::Callback<typename std::decay<F>::type,
      typename std::decay<T>::type>(m_keepAlive, std::forward<F>(callback),
      std::forward<T>(defaultValue));
  }
}
}

#endif
