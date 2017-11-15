#ifndef BEAM_VIRTUALTIMER_HPP
#define BEAM_VIRTUALTIMER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Threading/Timer.hpp"

namespace Beam {
namespace Threading {

  /*! \class VirtualTimer
      \brief Provides a virtual interface for a Timer.
   */
  class VirtualTimer : private boost::noncopyable {
    public:
      virtual ~VirtualTimer() = default;

      virtual void Start() = 0;

      virtual void Cancel() = 0;

      virtual void Wait() = 0;

      virtual const Publisher<Timer::Result>& GetPublisher() const = 0;

    protected:

      //! Constructs a VirtualTimer.
      VirtualTimer() = default;
  };

  /*! \class WrapperTimer
      \brief Wraps a Timer providing it with a virtual interface.
      \tparam TimerType The type of Timer to wrap.
   */
  template<typename TimerType>
  class WrapperTimer : public VirtualTimer {
    public:

      //! The Timer to wrap.
      using Timer = GetTryDereferenceType<TimerType>;

      //! Constructs a WrapperTimer.
      /*!
        \param timer The Timer to wrap.
      */
      template<typename TimerForward>
      WrapperTimer(TimerForward&& timer);

      //! Returns the Timer being wrapped.
      const Timer& GetTimer() const;

      //! Returns the Timer being wrapped.
      Timer& GetTimer();

      virtual void Start() override;

      virtual void Cancel() override;

      virtual void Wait() override;

      virtual const Publisher<Threading::Timer::Result>& GetPublisher()
        const override;

    private:
      GetOptionalLocalPtr<TimerType> m_timer;
  };

  //! Wraps a Timer into a VirtualTimer.
  /*!
    \param timer The Timer to wrap.
  */
  template<typename Timer>
  std::unique_ptr<VirtualTimer> MakeVirtualTimer(Timer&& timer) {
    return std::make_unique<WrapperTimer<typename std::decay<Timer>::type>>(
      std::forward<Timer>(timer));
  }

  template<typename TimerType>
  template<typename TimerForward>
  WrapperTimer<TimerType>::WrapperTimer(TimerForward&& timer)
      : m_timer{std::forward<TimerForward>(timer)} {}

  template<typename TimerType>
  const typename WrapperTimer<TimerType>::Timer&
      WrapperTimer<TimerType>::GetTimer() const {
    return *m_timer;
  }

  template<typename TimerType>
  typename WrapperTimer<TimerType>::Timer& WrapperTimer<TimerType>::GetTimer() {
    return *m_timer;
  }

  template<typename TimerType>
  void WrapperTimer<TimerType>::Start() {
    m_timer->Start();
  }

  template<typename TimerType>
  void WrapperTimer<TimerType>::Cancel() {
    m_timer->Cancel();
  }

  template<typename TimerType>
  void WrapperTimer<TimerType>::Wait() {
    m_timer->Wait();
  }

  template<typename TimerType>
  const Publisher<Threading::Timer::Result>& WrapperTimer<TimerType>::
      GetPublisher() const {
    return m_timer->GetPublisher();
  }
}

  template<>
  struct ImplementsConcept<Threading::VirtualTimer, Threading::Timer> :
    std::true_type {};
}

#endif
