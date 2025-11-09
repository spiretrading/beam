#ifndef BEAM_TEST_TIMER_HPP
#define BEAM_TEST_TIMER_HPP
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include "Beam/Pointers/Ref.hpp"
#include "Beam/TimeService/TriggerTimer.hpp"

namespace Beam::Tests {
  class TimeServiceTestEnvironment;

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
      mutable boost::mutex m_mutex;
      boost::posix_time::time_duration m_interval;
      TimeServiceTestEnvironment* m_environment;
      bool m_has_started;
      TriggerTimer m_timer;

      TestTimer(const TestTimer&) = delete;
      TestTimer& operator =(const TestTimer&) = delete;
      void trigger();
      void fail();
  };
}

#endif
