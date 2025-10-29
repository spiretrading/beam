#ifndef BEAM_TRIGGER_TIMER_HPP
#define BEAM_TRIGGER_TIMER_HPP
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/TimeService/Timer.hpp"

namespace Beam {

  /** A Timer that expires when explicitly triggered. */
  class TriggerTimer {
    public:
      using Result = Timer::Result;

      /** Constructs a TriggerTimer. */
      TriggerTimer();

      ~TriggerTimer();

      /** Triggers the Timer to expire. */
      void trigger();

      /** Causes the Timer to fail. */
      void fail();

      void start();
      void cancel();
      void wait();
      const Publisher<Timer::Result>& get_publisher() const;

    private:
      mutable Mutex m_mutex;
      int m_state;
      Timer::Result m_result;
      QueueWriterPublisher<Timer::Result> m_publisher;
      ConditionVariable m_trigger;

      TriggerTimer(const TriggerTimer&) = delete;
      TriggerTimer& operator =(const TriggerTimer&) = delete;
      void publish();
  };

  inline TriggerTimer::TriggerTimer()
    : m_state(0) {}

  inline TriggerTimer::~TriggerTimer() {
    cancel();
  }

  inline void TriggerTimer::trigger() {
    auto lock = boost::lock_guard(m_mutex);
    m_result = Timer::Result::EXPIRED;
    if(m_state == 0) {
      m_state = 2;
    } else if(m_state == 1) {
      publish();
    }
  }

  inline void TriggerTimer::fail() {
    auto lock = boost::lock_guard(m_mutex);
    m_result = Timer::Result::FAIL;
    if(m_state == 0) {
      m_state = 2;
    } else if(m_state == 1) {
      publish();
    }
  }

  inline void TriggerTimer::start() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_state == 0) {
      m_state = 1;
    } else if(m_state == 2) {
      publish();
    }
  }

  inline void TriggerTimer::cancel() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_state == 0) {
      m_state = 1;
    } else if(m_state == 1) {
      m_result = Timer::Result::CANCELED;
      publish();
    } else if(m_state == 2) {
      publish();
    }
  }

  inline void TriggerTimer::wait() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_state != 0) {
      m_trigger.wait(lock);
    }
  }

  inline const Publisher<Timer::Result>& TriggerTimer::get_publisher() const {
    return m_publisher;
  }

  inline void TriggerTimer::publish() {
    m_publisher.push(m_result);
    m_state = 0;
    m_trigger.notify_all();
  }
}

#endif
