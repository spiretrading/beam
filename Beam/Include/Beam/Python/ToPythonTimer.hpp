#ifndef BEAM_TO_PYTHON_TIMER_HPP
#define BEAM_TO_PYTHON_TIMER_HPP
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Threading/VirtualTimer.hpp"

namespace Beam {
namespace Threading {

  /*! \class ToPythonTimer
      \brief Wraps a Timer class for use within Python.
      \tparam TimerType The type of Timer to wrap.
   */
  template<typename TimerType>
  class ToPythonTimer : public VirtualTimer {
    public:

      //! The type of Timer to wrap.
      using Timer = TimerType;

      //! Constructs a ToPythonTimer.
      /*!
        \param timer The Timer to wrap.
      */
      ToPythonTimer(std::unique_ptr<Timer> timer);

      virtual ~ToPythonTimer() override final;

      //! Returns the wrapped Timer.
      const Timer& GetTimer() const;

      //! Returns the wrapped Timer.
      Timer& GetTimer();

      virtual void Start() override final;

      virtual void Cancel() override final;

      virtual void Wait() override final;

      virtual const Publisher<Threading::Timer::Result>&
        GetPublisher() const override final;

    private:
      std::unique_ptr<Timer> m_timer;
  };

  //! Makes a ToPythonTimer.
  /*!
    \param timer The Timer to wrap.
  */
  template<typename Timer>
  auto MakeToPythonTimer(std::unique_ptr<Timer> timer) {
    return std::make_shared<ToPythonTimer<Timer>>(std::move(timer));
  }

  template<typename TimerType>
  ToPythonTimer<TimerType>::ToPythonTimer(std::unique_ptr<Timer> timer)
      : m_timer{std::move(timer)} {}

  template<typename TimerType>
  ToPythonTimer<TimerType>::~ToPythonTimer() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_timer->Cancel();
    m_timer.reset();
  }

  template<typename TimerType>
  const typename ToPythonTimer<TimerType>::Timer&
      ToPythonTimer<TimerType>::GetTimer() const {
    return *m_timer;
  }

  template<typename TimerType>
  typename ToPythonTimer<TimerType>::Timer&
      ToPythonTimer<TimerType>::GetTimer() {
    return *m_timer;
  }

  template<typename TimerType>
  void ToPythonTimer<TimerType>::Start() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_timer->Start();
  }

  template<typename TimerType>
  void ToPythonTimer<TimerType>::Cancel() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_timer->Cancel();
  }

  template<typename TimerType>
  void ToPythonTimer<TimerType>::Wait() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_timer->Wait();
  }

  template<typename TimerType>
  const Publisher<Threading::Timer::Result>&
      ToPythonTimer<TimerType>::GetPublisher() const {
    return m_timer->GetPublisher();
  }
}
}

#endif
