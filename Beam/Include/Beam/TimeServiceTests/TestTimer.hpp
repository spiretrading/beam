#ifndef BEAM_TEST_TIMER_HPP
#define BEAM_TEST_TIMER_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"
#include "Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp"

namespace Beam::Tests {

  /** The type of Timer used by the TestEnvironment. */
  class TestTimer {
    public:
      using Result = Timer::Result;

      /**
       * Constructs a TestTimer.
       * @param interval The time interval before expiring.
       * @param environment The TestEnvironment this Timer belongs to.
       */
      TestTimer(boost::posix_time::time_duration interval,
        Ref<TimeServiceTestEnvironment> environment);

      ~TestTimer();

      void start();
      void cancel();
      void wait();
      const Publisher<Timer::Result>& get_publisher() const;

    private:
      friend class TimeServiceTestEnvironment;
      friend void fail(TestTimer& timer);
      friend void trigger(TestTimer& timer);
      mutable boost::mutex m_mutex;
      boost::posix_time::time_duration m_interval;
      TimeServiceTestEnvironment* m_environment;
      bool m_has_started;
      TriggerTimer m_timer;

      TestTimer(const TestTimer&) = delete;
      TestTimer& operator =(const TestTimer&) = delete;
  };

  inline TestTimer::TestTimer(boost::posix_time::time_duration interval,
    Ref<TimeServiceTestEnvironment> environment)
    : m_interval(interval),
      m_environment(environment.get()),
      m_has_started(false) {}

  inline TestTimer::~TestTimer() {
    cancel();
  }

  inline void TestTimer::start() {
    {
      auto lock = boost::lock_guard(m_mutex);
      if(m_has_started) {
        return;
      }
      m_has_started = true;
    }
    m_timer.start();
    m_environment->add(this);
  }

  inline void TestTimer::cancel() {
    {
      auto lock = boost::lock_guard(m_mutex);
      if(!m_has_started) {
        return;
      }
      m_has_started = false;
    }
    m_environment->remove(this);
    m_timer.cancel();
  }

  inline void TestTimer::wait() {
    m_timer.wait();
  }

  inline const Publisher<Timer::Result>& TestTimer::get_publisher() const {
    return m_timer.get_publisher();
  }

  inline void TimeServiceTestEnvironment::add(TestTimer* timer) {
    if(timer->m_interval <= boost::posix_time::seconds(0)) {
      timer->m_timer.trigger();
      return;
    }
    auto entry = TimerEntry(timer, timer->m_interval);
    auto lock = boost::lock_guard(m_mutex);
    m_timers.push_back(entry);
    m_next_trigger = std::min(m_next_trigger, timer->m_interval);
  }

  inline void fail(TestTimer& timer) {
    {
      auto lock = boost::lock_guard(timer.m_mutex);
      timer.m_has_started = false;
    }
    timer.m_timer.fail();
  }

  inline void trigger(TestTimer& timer) {
    {
      auto lock = boost::lock_guard(timer.m_mutex);
      timer.m_has_started = false;
    }
    timer.m_timer.trigger();
  }
}

#endif
