#ifndef BEAM_TRIGGER_TIMER_HPP
#define BEAM_TRIGGER_TIMER_HPP
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/Timer.hpp"

namespace Beam {
namespace Threading {

  /** A Timer that expires when explicitly triggered. */
  class TriggerTimer {
    public:

      /** Constructs a TriggerTimer. */
      TriggerTimer();

      ~TriggerTimer();

      /** Triggers the Timer to expire. */
      void Trigger();

      /** Causes the Timer to fail. */
      void Fail();

      void Start();

      void Cancel();

      void Wait();

      const Publisher<Timer::Result>& GetPublisher() const;

    private:
      mutable Mutex m_mutex;
      int m_state;
      Timer::Result m_result;
      QueueWriterPublisher<Timer::Result> m_publisher;
      ConditionVariable m_trigger;

      TriggerTimer(const TriggerTimer&) = delete;
      TriggerTimer& operator =(const TriggerTimer&) = delete;
      void Publish();
  };

  inline TriggerTimer::TriggerTimer()
    : m_state(0) {}

  inline TriggerTimer::~TriggerTimer() {
    Cancel();
  }

  inline void TriggerTimer::Trigger() {
    auto lock = boost::lock_guard(m_mutex);
    m_result = Timer::Result::EXPIRED;
    if(m_state == 0) {
      m_state = 2;
    } else if(m_state == 1) {
      Publish();
    }
  }

  inline void TriggerTimer::Fail() {
    auto lock = boost::lock_guard(m_mutex);
    m_result = Timer::Result::FAIL;
    if(m_state == 0) {
      m_state = 2;
    } else if(m_state == 1) {
      Publish();
    }
  }

  inline void TriggerTimer::Start() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_state == 0) {
      m_state = 1;
    } else if(m_state == 2) {
      Publish();
    }
  }

  inline void TriggerTimer::Cancel() {
    auto lock = boost::lock_guard(m_mutex);
    if(m_state == 0) {
      m_state = 1;
    } else if(m_state == 1) {
      m_result = Timer::Result::CANCELED;
      Publish();
    } else if(m_state == 2) {
      Publish();
    }
  }

  inline void TriggerTimer::Wait() {
    auto lock = boost::unique_lock(m_mutex);
    while(m_state != 0) {
      m_trigger.wait(lock);
    }
  }

  inline const Publisher<Timer::Result>& TriggerTimer::GetPublisher() const {
    return m_publisher;
  }

  inline void TriggerTimer::Publish() {
    m_publisher.Push(m_result);
    m_state = 0;
    m_trigger.notify_all();
  }
}

  template<>
  struct ImplementsConcept<Threading::TriggerTimer, Threading::Timer> :
    std::true_type {};
}

#endif
