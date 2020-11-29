#ifndef BEAM_TO_PYTHON_TIMER_HPP
#define BEAM_TO_PYTHON_TIMER_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Threading/Timer.hpp"

namespace Beam::Threading {

  /**
   * Wraps a Timer class for use within Python.
   * @param <T> The type of Timer to wrap.
   */
  template<typename T>
  class ToPythonTimer {
    public:

      /** The type of Timer to wrap. */
      using Timer = T;

      /**
       * Constructs a ToPythonTimer in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      ToPythonTimer(Args&&... args);

      ToPythonTimer(ToPythonTimer&&) = default;

      ~ToPythonTimer();

      /** Returns the wrapped Timer. */
      const Timer& GetTimer() const;

      /** Returns the wrapped Timer. */
      Timer& GetTimer();

      void Start();

      void Cancel();

      void Wait();

      const Publisher<Threading::Timer::Result>& GetPublisher() const;

      ToPythonTimer& operator =(ToPythonTimer&&) = default;

    private:
      boost::optional<Timer> m_timer;

      ToPythonTimer(const ToPythonTimer&) = delete;
      ToPythonTimer& operator =(const ToPythonTimer&) = delete;
  };

  template<typename Timer>
  ToPythonTimer(Timer&&) -> ToPythonTimer<std::decay_t<Timer>>;

  template<typename T>
  template<typename... Args>
  ToPythonTimer<T>::ToPythonTimer(Args&&... args)
    : m_timer((Python::GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<typename C>
  ToPythonTimer<C>::~ToPythonTimer() {
    auto release = Python::GilRelease();
    m_timer.reset();
  }

  template<typename T>
  const typename ToPythonTimer<T>::Timer& ToPythonTimer<T>::GetTimer() const {
    return *m_timer;
  }

  template<typename T>
  typename ToPythonTimer<T>::Timer& ToPythonTimer<T>::GetTimer() {
    return *m_timer;
  }

  template<typename T>
  void ToPythonTimer<T>::Start() {
    auto release = Python::GilRelease();
    m_timer->Start();
  }

  template<typename T>
  void ToPythonTimer<T>::Cancel() {
    auto release = Python::GilRelease();
    m_timer->Cancel();
  }

  template<typename T>
  void ToPythonTimer<T>::Wait() {
    auto release = Python::GilRelease();
    m_timer->Wait();
  }

  template<typename T>
  const Publisher<Threading::Timer::Result>&
      ToPythonTimer<T>::GetPublisher() const {
    return m_timer->GetPublisher();
  }
}

#endif
