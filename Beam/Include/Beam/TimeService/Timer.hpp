#ifndef BEAM_TIMER_HPP
#define BEAM_TIMER_HPP
#include <concepts>
#include <memory>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {
namespace Details {
  BEAM_ENUM(TimerResult,

    /** The Timer expired. */
    EXPIRED,

    /** The Timer was canceled. */
    CANCELED,

    /** The Timer encountered an error. */
    FAIL);
}

  /** Concept that matches the Timer interface. */
  template<typename T>
  concept IsTimer = requires(T& t, const T& ct) {
    requires std::same_as<typename T::Result, Details::TimerResult>;
    { t.start() } -> std::same_as<void>;
    { t.cancel() } -> std::same_as<void>;
    { t.wait() } -> std::same_as<void>;
    { ct.get_publisher() } ->
      std::same_as<const Publisher<typename T::Result>&>;
  };

  class Timer {
    public:

      /** Enumerates the result of a Timer expiry. */
      using Result = Details::TimerResult;

      /**
       * Constructs a Timer of a specified type using emplacement.
       * @tparam T The type of timer to emplace.
       * @param args The arguments to pass to the emplaced timer.
       */
      template<IsTimer T, typename... Args>
      explicit Timer(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a Timer by referencing an existing timer.
       * @param timer The timer to reference.
       */
      template<DisableCopy<Timer> T> requires IsTimer<dereference_t<T>>
      Timer(T&& timer);

      Timer(const Timer&) = default;
      Timer(Timer&&) = default;

      /** Starts the Timer. */
      void start();

      /** Cancels the Timer. */
      void cancel();

      /** Waits for the Timer to expire. */
      void wait();

      /** Returns the object publishing the result of a start. */
      const Publisher<Timer::Result>& get_publisher() const;

    private:
      struct VirtualTimer {
        virtual ~VirtualTimer() = default;

        virtual void start() = 0;
        virtual void cancel() = 0;
        virtual void wait() = 0;
        virtual const Publisher<Timer::Result>& get_publisher() const = 0;
      };
      template<typename T>
      struct WrappedTimer final : VirtualTimer {
        using TimerType = T;
        local_ptr_t<T> m_timer;

        template<typename... Args>
        WrappedTimer(Args&&... args);

        void start() override;
        void cancel() override;
        void wait() override;
        const Publisher<Timer::Result>& get_publisher() const override;
      };
      VirtualPtr<VirtualTimer> m_timer;
  };

  template<IsTimer T, typename... Args>
  Timer::Timer(std::in_place_type_t<T>, Args&&... args)
    : m_timer(make_virtual_ptr<WrappedTimer<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<Timer> T> requires IsTimer<dereference_t<T>>
  Timer::Timer(T&& timer)
    : m_timer(make_virtual_ptr<WrappedTimer<std::remove_cvref_t<T>>>(
        std::forward<T>(timer))) {}

  inline void Timer::start() {
    m_timer->start();
  }

  inline void Timer::cancel() {
    m_timer->cancel();
  }

  inline void Timer::wait() {
    m_timer->wait();
  }

  inline const Publisher<Timer::Result>& Timer::get_publisher() const {
    return m_timer->get_publisher();
  }

  template<typename T>
  template<typename... Args>
  Timer::WrappedTimer<T>::WrappedTimer(Args&&... args)
    : m_timer(std::forward<Args>(args)...) {}

  template<typename T>
  void Timer::WrappedTimer<T>::start() {
    m_timer->start();
  }

  template<typename T>
  void Timer::WrappedTimer<T>::cancel() {
    m_timer->cancel();
  }

  template<typename T>
  void Timer::WrappedTimer<T>::wait() {
    m_timer->wait();
  }

  template<typename T>
  const Publisher<Timer::Result>&
      Timer::WrappedTimer<T>::get_publisher() const {
    return m_timer->get_publisher();
  }
}

#endif
