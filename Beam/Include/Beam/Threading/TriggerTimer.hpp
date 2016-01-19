#ifndef BEAM_TRIGGERTIMER_HPP
#define BEAM_TRIGGERTIMER_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Threading/ConditionVariable.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/Threading/Timer.hpp"

namespace Beam {
namespace Threading {

  /*! \class TriggerTimer
      \brief A Timer that expires when explicitly triggered.
   */
  class TriggerTimer : private boost::noncopyable {
    public:

      //! Constructs a TriggerTimer.
      TriggerTimer();

      ~TriggerTimer();

      //! Triggers the Timer to expire.
      void Trigger();

      void Start();

      void Cancel();

      void Wait();

      const Publisher<Timer::Result>& GetPublisher() const;

    private:
      mutable Mutex m_mutex;
      bool m_isPending;
      MultiQueueWriter<Timer::Result> m_publisher;
      ConditionVariable m_trigger;
  };

  inline TriggerTimer::TriggerTimer()
    : m_isPending(false) {}

  inline TriggerTimer::~TriggerTimer() {
    Cancel();
  }

  inline void TriggerTimer::Trigger() {
    boost::lock_guard<Mutex> lock(m_mutex);
    m_isPending = false;
    m_publisher.Push(Timer::Result::EXPIRED);
    m_trigger.notify_all();
  }

  inline void TriggerTimer::Start() {
    boost::lock_guard<Mutex> lock(m_mutex);
    if(m_isPending) {
      return;
    }
    m_isPending = true;
  }

  inline void TriggerTimer::Cancel() {
    boost::lock_guard<Mutex> lock(m_mutex);
    if(!m_isPending) {
      return;
    }
    m_isPending = false;
    m_publisher.Push(Timer::Result::CANCELED);
    m_trigger.notify_all();
  }

  inline void TriggerTimer::Wait() {
    boost::unique_lock<Mutex> lock(m_mutex);
    while(m_isPending) {
      m_trigger.wait(lock);
    }
  }

  inline const Publisher<Timer::Result>& TriggerTimer::GetPublisher() const {
    return m_publisher;
  }
}

  template<>
  struct ImplementsConcept<Threading::TriggerTimer, Threading::Timer> :
    std::true_type {};
}

#endif
