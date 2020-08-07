#ifndef BEAM_LIVETIMER_HPP
#define BEAM_LIVETIMER_HPP
#include <boost/asio/deadline_timer.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/system_error.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/Timer.hpp"
#include "Beam/Threading/TimerThreadPool.hpp"

namespace Beam {
namespace Threading {

  /*! \class LiveTimer
      \brief Implements a Timer using a live timer.
   */
  class LiveTimer : private boost::noncopyable {
    public:

      //! Constructs a LiveTimer.
      /*!
        \param interval The time interval before expiring.
        \param timerThreadPool The thread pool used to trigger the timer's
               expired slot.
      */
      LiveTimer(boost::posix_time::time_duration interval,
        Ref<TimerThreadPool> timerThreadPool);

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
  };

  inline LiveTimer::LiveTimer(boost::posix_time::time_duration interval,
    Ref<TimerThreadPool> timerThreadPool)
      : m_interval(interval),
        m_deadLineTimer(timerThreadPool->GetService()),
        m_isPending(false) {}

  inline LiveTimer::~LiveTimer() {
    Cancel();
  }

  inline void LiveTimer::Start() {
    boost::lock_guard<Mutex> lock(m_mutex);
    if(m_isPending) {
      return;
    }
    m_isPending = true;
    m_deadLineTimer.expires_from_now(m_interval);
    m_deadLineTimer.async_wait(
      [=] (const boost::system::error_code& error) {
        boost::lock_guard<Mutex> lock(m_mutex);
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
    boost::unique_lock<Mutex> lock(m_mutex);
    if(m_isPending) {
      m_deadLineTimer.cancel();
      while(m_isPending) {
        m_trigger.wait(lock);
      }
    }
  }

  inline void LiveTimer::Wait() {
    boost::unique_lock<Mutex> lock(m_mutex);
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
