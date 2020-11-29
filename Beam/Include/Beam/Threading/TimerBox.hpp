#ifndef BEAM_TIMER_BOX_HPP
#define BEAM_TIMER_BOX_HPP
#include <memory>
#include <type_traits>
#include "Beam/Threading/Threading.hpp"
#include "Beam/Threading/Timer.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace Threading {

  /** Provides a generic interface over an arbitrary Timer. */
  class TimerBox {
    public:

      /**
       * Constructs a TimerBox of a specified type using emplacement.
       * @param <T> The type of timer to emplace.
       * @param args The arguments to pass to the emplaced timer.
       */
      template<typename T, typename... Args>
      explicit TimerBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a TimerBox by copying an existing timer.
       * @param timer The timer to copy.
       */
      template<typename Timer>
      explicit TimerBox(Timer timer);

      explicit TimerBox(TimerBox* timer);

      explicit TimerBox(const std::shared_ptr<TimerBox>& timer);

      explicit TimerBox(const std::unique_ptr<TimerBox>& timer);

      void Start();

      void Cancel();

      void Wait();

      const Publisher<Timer::Result>& GetPublisher() const;

    private:
      struct VirtualTimer {
        virtual ~VirtualTimer() = default;
        virtual void Start() = 0;
        virtual void Cancel() = 0;
        virtual void Wait() = 0;
        virtual const Publisher<Threading::Timer::Result>&
          GetPublisher() const = 0;
      };
      template<typename T>
      struct WrappedTimer final : VirtualTimer {
        using Timer = T;
        GetOptionalLocalPtr<Timer> m_timer;

        template<typename... Args>
        WrappedTimer(Args&&... args);
        void Start() override;
        void Cancel() override;
        void Wait() override;
        const Publisher<Threading::Timer::Result>&
          GetPublisher() const override;
      };
      std::shared_ptr<VirtualTimer> m_timer;
  };

  template<typename T, typename... Args>
  TimerBox::TimerBox(std::in_place_type_t<T>, Args&&... args)
    : m_timer(std::make_shared<WrappedTimer<T>>(std::forward<Args>(args)...)) {}

  template<typename Timer>
  TimerBox::TimerBox(Timer timer)
    : TimerBox(std::in_place_type<Timer>, std::move(timer)) {}

  inline TimerBox::TimerBox(TimerBox* timer)
    : TimerBox(*timer) {}

  inline TimerBox::TimerBox(const std::shared_ptr<TimerBox>& timer)
    : TimerBox(*timer) {}

  inline TimerBox::TimerBox(const std::unique_ptr<TimerBox>& timer)
    : TimerBox(*timer) {}

  inline void TimerBox::Start() {
    m_timer->Start();
  }

  inline void TimerBox::Cancel() {
    m_timer->Cancel();
  }

  inline void TimerBox::Wait() {
    m_timer->Wait();
  }

  inline const Publisher<Timer::Result>& TimerBox::GetPublisher() const {
    return m_timer->GetPublisher();
  }

  template<typename T>
  template<typename... Args>
  TimerBox::WrappedTimer<T>::WrappedTimer(Args&&... args)
    : m_timer(std::forward<Args>(args)...) {}

  template<typename T>
  void TimerBox::WrappedTimer<T>::Start() {
    m_timer->Start();
  }

  template<typename T>
  void TimerBox::WrappedTimer<T>::Cancel() {
    m_timer->Cancel();
  }

  template<typename T>
  void TimerBox::WrappedTimer<T>::Wait() {
    m_timer->Wait();
  }

  template<typename T>
  const Publisher<Threading::Timer::Result>&
      TimerBox::WrappedTimer<T>::GetPublisher() const {
    return m_timer->GetPublisher();
  }
}

  template<>
  struct ImplementsConcept<Threading::TimerBox, Threading::Timer> :
    std::true_type {};
}

#endif
