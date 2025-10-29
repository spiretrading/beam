#ifndef BEAM_LIVE_TIMER_HPP
#define BEAM_LIVE_TIMER_HPP
#include <boost/asio/deadline_timer.hpp>
#include <boost/system/system_error.hpp>
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/ServiceThreadPool.hpp"
#include "Beam/TimeService/Timer.hpp"

namespace Beam {

  /** Implements a Timer using a live timer. */
  class LiveTimer {
    public:
      using Result = Timer::Result;

      /**
       * Constructs a LiveTimer.
       * @param interval The time interval before expiring.
       */
      explicit LiveTimer(boost::posix_time::time_duration interval);

      ~LiveTimer();

      void start();
      void cancel();
      void wait();
      const Publisher<Timer::Result>& get_publisher() const;

    private:
      mutable Mutex m_mutex;
      boost::posix_time::time_duration m_interval;
      boost::asio::deadline_timer m_deadline_timer;
      bool m_is_pending;
      QueueWriterPublisher<Timer::Result> m_publisher;
      ConditionVariable m_trigger;

      LiveTimer(const LiveTimer&) = delete;
      LiveTimer& operator =(const LiveTimer&) = delete;
  };

  inline LiveTimer::LiveTimer(boost::posix_time::time_duration interval)
    : m_interval(interval),
      m_deadline_timer(ServiceThreadPool::get().get_context()),
      m_is_pending(false) {}

  inline LiveTimer::~LiveTimer() {
    cancel();
  }

  inline void LiveTimer::start() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_is_pending) {
      return;
    }
    m_is_pending = true;
    m_deadline_timer.expires_from_now(m_interval);
    m_deadline_timer.async_wait([this] (const auto& error) {
      auto lock = boost::lock_guard(m_mutex);
      if(error) {
        m_publisher.push(Timer::Result::CANCELED);
      } else {
        m_publisher.push(Timer::Result::EXPIRED);
      }
      m_is_pending = false;
      m_trigger.notify_all();
    });
  }

  inline void LiveTimer::cancel() {
    auto lock = boost::unique_lock(m_mutex);
    if(m_is_pending) {
      m_deadline_timer.cancel();
      while(m_is_pending) {
        m_trigger.wait(lock);
      }
    }
  }

  inline void LiveTimer::wait() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_is_pending) {
      m_trigger.wait(lock);
    }
  }

  inline const Publisher<Timer::Result>& LiveTimer::get_publisher() const {
    return m_publisher;
  }
}

#endif
