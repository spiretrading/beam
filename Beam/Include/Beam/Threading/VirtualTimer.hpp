#ifndef BEAM_VIRTUAL_TIMER_HPP
#define BEAM_VIRTUAL_TIMER_HPP
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Threading/Timer.hpp"

namespace Beam {
namespace Threading {

  /** Provides a virtual interface for a Timer. */
  class VirtualTimer {
    public:
      virtual ~VirtualTimer() = default;

      virtual void Start() = 0;

      virtual void Cancel() = 0;

      virtual void Wait() = 0;

      virtual const Publisher<Timer::Result>& GetPublisher() const = 0;

    protected:

      /** Constructs a VirtualTimer. */
      VirtualTimer() = default;

    private:
      VirtualTimer(const VirtualTimer&) = delete;
      VirtualTimer& operator =(const VirtualTimer&) = delete;
  };

  /**
   * Wraps a Timer providing it with a virtual interface.
   * @param <T> The type of Timer to wrap.
   */
  template<typename T>
  class WrapperTimer : public VirtualTimer {
    public:

      /** The Timer to wrap. */
      using Timer = GetTryDereferenceType<T>;

      /**
       * Constructs a WrapperTimer.
       * @param timer The Timer to wrap.
       */
      template<typename TF>
      WrapperTimer(TF&& timer);

      /** Returns the Timer being wrapped. */
      const Timer& GetTimer() const;

      /** Returns the Timer being wrapped. */
      Timer& GetTimer();

      void Start() override;

      void Cancel() override;

      void Wait() override;

      const Publisher<Threading::Timer::Result>& GetPublisher() const override;

    private:
      GetOptionalLocalPtr<T> m_timer;
  };

  /**
   * Wraps a Timer into a VirtualTimer.
   * @param timer The Timer to wrap.
   */
  template<typename Timer>
  std::unique_ptr<VirtualTimer> MakeVirtualTimer(Timer&& timer) {
    return std::make_unique<WrapperTimer<std::decay_t<Timer>>>(
      std::forward<Timer>(timer));
  }

  template<typename T>
  template<typename TF>
  WrapperTimer<T>::WrapperTimer(TF&& timer)
    : m_timer(std::forward<TF>(timer)) {}

  template<typename T>
  const typename WrapperTimer<T>::Timer& WrapperTimer<T>::GetTimer() const {
    return *m_timer;
  }

  template<typename T>
  typename WrapperTimer<T>::Timer& WrapperTimer<T>::GetTimer() {
    return *m_timer;
  }

  template<typename T>
  void WrapperTimer<T>::Start() {
    m_timer->Start();
  }

  template<typename T>
  void WrapperTimer<T>::Cancel() {
    m_timer->Cancel();
  }

  template<typename T>
  void WrapperTimer<T>::Wait() {
    m_timer->Wait();
  }

  template<typename T>
  const Publisher<Threading::Timer::Result>&
      WrapperTimer<T>::GetPublisher() const {
    return m_timer->GetPublisher();
  }
}

  template<>
  struct ImplementsConcept<Threading::VirtualTimer, Threading::Timer> :
    std::true_type {};
}

#endif
