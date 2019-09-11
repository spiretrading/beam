#ifndef BEAM_TO_PYTHON_TIMER_HPP
#define BEAM_TO_PYTHON_TIMER_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Threading/VirtualTimer.hpp"

namespace Beam::Threading {

  /**
   * Wraps a Timer class for use within Python.
   * @param <T> The type of Timer to wrap.
   */
  template<typename T>
  class ToPythonTimer final : public VirtualTimer {
    public:

      /** The type of Timer to wrap. */
      using Timer = T;

      /**
       * Constructs a ToPythonTimer.
       * @param timer The Timer to wrap.
       */
      ToPythonTimer(std::unique_ptr<Timer> timer);

      ~ToPythonTimer() override;

      /** Returns the wrapped Timer. */
      const Timer& GetTimer() const;

      /** Returns the wrapped Timer. */
      Timer& GetTimer();

      void Start() override;

      void Cancel() override;

      void Wait() override;

      const Publisher<Threading::Timer::Result>& GetPublisher() const override;

    private:
      std::unique_ptr<Timer> m_timer;
  };

  /**
   * Makes a ToPythonTimer.
   * @param timer The Timer to wrap.
   */
  template<typename Timer>
  auto MakeToPythonTimer(std::unique_ptr<Timer> timer) {
    return std::make_unique<ToPythonTimer<Timer>>(std::move(timer));
  }

  template<typename T>
  ToPythonTimer<T>::ToPythonTimer(std::unique_ptr<Timer> timer)
    : m_timer(std::move(timer)) {}

  template<typename T>
  ToPythonTimer<T>::~ToPythonTimer() {
    m_timer->Cancel();
    auto release = Python::GilRelease();
    m_timer.reset();
  }

  template<typename T>
  const typename ToPythonTimer<T>::Timer&
      ToPythonTimer<T>::GetTimer() const {
    return *m_timer;
  }

  template<typename T>
  typename ToPythonTimer<T>::Timer&
      ToPythonTimer<T>::GetTimer() {
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
