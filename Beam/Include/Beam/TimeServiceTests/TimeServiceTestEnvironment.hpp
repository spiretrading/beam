#ifndef BEAM_TIME_SERVICE_TEST_ENVIRONMENT_HPP
#define BEAM_TIME_SERVICE_TEST_ENVIRONMENT_HPP
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/TimeServiceTests/TimeServiceTestEnvironmentException.hpp"
#include "Beam/TimeServiceTests/TimeServiceTests.hpp"

namespace Beam::TimeService::Tests {
  void Fail(TestTimer& timer);
  void Trigger(TestTimer& timer);

  /** Simulates the passing of time. */
  class TimeServiceTestEnvironment {
    public:

      /** Constructs a TimeServiceTestEnvironment using the system time. */
      TimeServiceTestEnvironment();

      /**
       * Constructs a TimeServiceTestEnvironment.
       * @param time The time to set the environment to.
       */
      TimeServiceTestEnvironment(boost::posix_time::ptime time);

      ~TimeServiceTestEnvironment();

      /**
       * Sets the time.
       * @param time The time to set the environment to.
       */
      void SetTime(boost::posix_time::ptime time);

      /**
       * Advances the time by a certain amount.
       * @param duration The amount of time to advance the environment by.
       */
      void AdvanceTime(boost::posix_time::time_duration duration);

      /** Returns the time. */
      boost::posix_time::ptime GetTime() const;

      void Close();

    private:
      struct TimerEntry {
        TestTimer* m_timer;
        boost::posix_time::time_duration m_timeRemaining;
      };
      friend class TestTimeClient;
      friend class TestTimer;
      mutable Threading::Mutex m_mutex;
      boost::posix_time::ptime m_currentTime;
      boost::posix_time::time_duration m_nextTrigger;
      SynchronizedVector<TestTimeClient*> m_timeClients;
      SynchronizedVector<TimerEntry> m_timers;
      IO::OpenState m_openState;

      TimeServiceTestEnvironment(const TimeServiceTestEnvironment&) = delete;
      TimeServiceTestEnvironment& operator =(
        const TimeServiceTestEnvironment&) = delete;
      void LockedSetTime(boost::posix_time::ptime time,
        boost::unique_lock<Threading::Mutex>& lock);
      void Add(TestTimeClient* timeClient);
      void Remove(TestTimeClient* timeClient);
      void Add(TestTimer* timer);
      void Remove(TestTimer* timer);
  };

  inline TimeServiceTestEnvironment::TimeServiceTestEnvironment()
    : TimeServiceTestEnvironment(
        boost::posix_time::second_clock::universal_time()) {}

  inline TimeServiceTestEnvironment::TimeServiceTestEnvironment(
      boost::posix_time::ptime time)
      : m_nextTrigger(boost::posix_time::pos_infin) {
    try {
      SetTime(time);
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  inline TimeServiceTestEnvironment::~TimeServiceTestEnvironment() {
    Close();
  }

  inline void TimeServiceTestEnvironment::SetTime(
      boost::posix_time::ptime time) {
    if(time.is_special()) {
      BOOST_THROW_EXCEPTION(
        TimeServiceTestEnvironmentException("Invalid date/time."));
    }
    Routines::FlushPendingRoutines();
    auto lock = boost::unique_lock(m_mutex);
    LockedSetTime(time, lock);
  }

  inline void TimeServiceTestEnvironment::AdvanceTime(
      boost::posix_time::time_duration duration) {
    Routines::FlushPendingRoutines();
    auto lock = boost::unique_lock(m_mutex);
    if(m_currentTime == boost::posix_time::not_a_date_time) {
      m_currentTime = boost::posix_time::ptime(
        boost::gregorian::date(2016, 8, 14), boost::posix_time::seconds(0));
    }
    LockedSetTime(m_currentTime + duration, lock);
  }

  inline boost::posix_time::ptime TimeServiceTestEnvironment::GetTime() const {
    auto lock = boost::unique_lock(m_mutex);
    return m_currentTime;
  }

  inline void TimeServiceTestEnvironment::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    auto pendingTimers = m_timers.Acquire();
    for(auto& pendingTimer : pendingTimers) {
      Fail(*pendingTimer.m_timer);
    }
    m_openState.Close();
    Routines::FlushPendingRoutines();
  }

  inline void TimeServiceTestEnvironment::LockedSetTime(
      boost::posix_time::ptime time,
      boost::unique_lock<Threading::Mutex>& lock) {
    if(m_currentTime != boost::posix_time::not_a_date_time &&
        m_currentTime >= time) {
      return;
    }
    auto delta = [&] {
      if(m_currentTime == boost::posix_time::not_a_date_time) {
        return boost::posix_time::time_duration{};
      } else {
        return time - m_currentTime;
      }
    }();
    while(delta > m_nextTrigger) {
      delta -= m_nextTrigger;
      LockedSetTime(m_currentTime + m_nextTrigger, lock);
    }
    m_nextTrigger -= delta;
    if(m_nextTrigger <= boost::posix_time::seconds(0)) {
      m_nextTrigger = boost::posix_time::pos_infin;
    }
    m_currentTime = time;
    m_timeClients.ForEach([&] (auto& timeClient) {
      timeClient->SetTime(time);
    });
    auto expiredTimers = std::vector<TestTimer*>();
    m_timers.With([&] (auto& timers) {
      timers.erase(std::remove_if(timers.begin(), timers.end(),
        [&] (auto& timer) {
          timer.m_timeRemaining -= delta;
          if(timer.m_timeRemaining <= boost::posix_time::seconds(0)) {
            expiredTimers.push_back(timer.m_timer);
            return true;
          } else {
            m_nextTrigger = std::min(m_nextTrigger, timer.m_timeRemaining);
            return false;
          }
        }), timers.end());
    });
    {
      auto release = Threading::Release(lock);
      for(auto& expiredTimer : expiredTimers) {
        Trigger(*expiredTimer);
      }
      Routines::FlushPendingRoutines();
    }
  }

  inline void TimeServiceTestEnvironment::Remove(TestTimeClient* timeClient) {
    m_timeClients.Remove(timeClient);
  }

  inline void TimeServiceTestEnvironment::Remove(TestTimer* timer) {
    m_timers.RemoveIf([&] (auto& entry) {
      return entry.m_timer == timer;
    });
  }
}

#endif
