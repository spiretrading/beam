#ifndef BEAM_TO_PYTHON_TIMER_HPP
#define BEAM_TO_PYTHON_TIMER_HPP
#include <type_traits>
#include <utility>
#include <boost/optional/optional.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/TimeService/Timer.hpp"

namespace Beam::Python {

  /**
   * Wraps a Timer class for use within Python.
   * @tparam T The type of Timer to wrap.
   */
  template<IsTimer T>
  class ToPythonTimer {
    public:

      /** The type of Timer to wrap. */
      using Timer = T;

      using Result = Beam::Timer::Result;

      /**
       * Constructs a ToPythonTimer in-place.
       * @param args The arguments to forward to the constructor.
       */
      template<typename... Args>
      explicit ToPythonTimer(Args&&... args);

      ~ToPythonTimer();

      /** Returns a reference to the underlying timer. */
      Timer& get();

      /** Returns a reference to the underlying timer. */
      const Timer& get() const;

      void start();
      void cancel();
      void wait();
      const Publisher<Beam::Timer::Result>& get_publisher() const;

    private:
      boost::optional<Timer> m_timer;

      ToPythonTimer(const ToPythonTimer&) = delete;
      ToPythonTimer& operator =(const ToPythonTimer&) = delete;
  };

  template<typename Timer>
  ToPythonTimer(Timer&&) -> ToPythonTimer<std::remove_cvref_t<Timer>>;

  template<IsTimer T>
  template<typename... Args>
  ToPythonTimer<T>::ToPythonTimer(Args&&... args)
    : m_timer((GilRelease(), boost::in_place_init),
        std::forward<Args>(args)...) {}

  template<IsTimer T>
  ToPythonTimer<T>::~ToPythonTimer() {
    auto release = GilRelease();
    m_timer.reset();
  }

  template<IsTimer T>
  typename ToPythonTimer<T>::Timer& ToPythonTimer<T>::get() {
    return *m_timer;
  }

  template<IsTimer T>
  const typename ToPythonTimer<T>::Timer& ToPythonTimer<T>::get() const {
    return *m_timer;
  }

  template<IsTimer T>
  void ToPythonTimer<T>::start() {
    auto release = GilRelease();
    m_timer->start();
  }

  template<IsTimer T>
  void ToPythonTimer<T>::cancel() {
    auto release = GilRelease();
    m_timer->cancel();
  }

  template<IsTimer T>
  void ToPythonTimer<T>::wait() {
    auto release = GilRelease();
    m_timer->wait();
  }

  template<IsTimer T>
  const Publisher<Beam::Timer::Result>&
      ToPythonTimer<T>::get_publisher() const {
    return m_timer->get_publisher();
  }
}

#endif
