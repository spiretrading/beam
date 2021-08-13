#ifndef BEAM_LIVE_TIMER_HPP
#define BEAM_LIVE_TIMER_HPP
#include <boost/asio/deadline_timer.hpp>
#include <boost/system/system_error.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/ServiceThreadPool.hpp"
#include "Beam/Threading/Timer.hpp"

namespace Beam {
namespace Threading {

  /** Implements a Timer using a live timer. */
  class LiveTimer {
    public:

      /**
       * Constructs a LiveTimer.
       * @param interval The time interval before expiring.
       */
      LiveTimer(boost::posix_time::time_duration interval);

      ~LiveTimer();

      void Start();

      void Cancel();

      void Wait();

      const Publisher<Timer::Result>& GetPublisher() const;

    private:
      mutable Mutex m_mutex;
      boost::posix_time::time_duration m_interval;
      boost::asio::deadline_timer m_deadLineTimer;
      bool m_isPending;
      QueueWriterPublisher<Timer::Result> m_publisher;
      ConditionVariable m_trigger;

      LiveTimer(const LiveTimer&) = delete;
      LiveTimer& operator =(const LiveTimer&) = delete;
  };

  inline LiveTimer::LiveTimer(boost::posix_time::time_duration interval)
    : m_interval(interval),
      m_deadLineTimer(ServiceThreadPool().GetInstance().GetService()),
      m_isPending(false) {}

  inline LiveTimer::~LiveTimer() {
    Cancel();
  }

  inline void LiveTimer::Start() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_isPending) {
      return;
    }
    m_isPending = true;
    m_deadLineTimer.expires_from_now(m_interval);
    m_deadLineTimer.async_wait([this] (const auto& error) {
      auto lock = boost::lock_guard(m_mutex);
      if(error) {
        m_publisher.Push(Timer::Result::CANCELED);
      } else {
        m_publisher.Push(Timer::Result::EXPIRED);
      }
      m_isPending = false;
      m_trigger.notify_all();
    });
  }

  inline void LiveTimer::Cancel() {
    auto lock = boost::unique_lock(m_mutex);
    if(m_isPending) {
      m_deadLineTimer.cancel();
      while(m_isPending) {
        m_trigger.wait(lock);
      }
    }
  }

  inline void LiveTimer::Wait() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_isPending) {
      m_trigger.wait(lock);
    }
  }

  inline const Publisher<Timer::Result>& LiveTimer::GetPublisher() const {
    return m_publisher;
  }
}

  template<>
  struct ImplementsConcept<Threading::LiveTimer, Threading::Timer> :
    std::true_type {};
}

#endif
