#ifndef BEAM_TIME_SERVICE_TEST_ENVIRONMENT_HPP
#define BEAM_TIME_SERVICE_TEST_ENVIRONMENT_HPP
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Collections/SynchronizedList.hpp"
#include "Beam/IO/OpenState.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Threading/Mutex.hpp"
#include "Beam/TimeServiceTests/TimeServiceTestEnvironmentException.hpp"
#include "Beam/TimeServiceTests/TestTimeClient.hpp"
#include "Beam/TimeServiceTests/TestTimer.hpp"

namespace Beam::Tests {

  /** Simulates the passing of time. */
  class TimeServiceTestEnvironment {
    public:

      /** Constructs a TimeServiceTestEnvironment using the system time. */
      TimeServiceTestEnvironment();

      /**
       * Constructs a TimeServiceTestEnvironment.
       * @param time The time to set the environment to.
       */
      explicit TimeServiceTestEnvironment(boost::posix_time::ptime time);

      ~TimeServiceTestEnvironment();

      /**
       * Sets the time.
       * @param time The time to set the environment to.
       */
      void set(boost::posix_time::ptime time);

      /**
       * Advances the time by a certain amount.
       * @param duration The amount of time to advance the environment by.
       */
      void advance(boost::posix_time::time_duration duration);

      /** Returns the time. */
      boost::posix_time::ptime get_time() const;

      void close();

    private:
      struct TimerEntry {
        TestTimer* m_timer;
        boost::posix_time::time_duration m_time_remaining;
      };
      friend class TestTimeClient;
      friend class TestTimer;
      mutable Mutex m_mutex;
      boost::posix_time::ptime m_current_time;
      boost::posix_time::time_duration m_next_trigger;
      SynchronizedVector<TestTimeClient*> m_time_clients;
      SynchronizedVector<TimerEntry> m_timers;
      OpenState m_open_state;

      TimeServiceTestEnvironment(const TimeServiceTestEnvironment&) = delete;
      TimeServiceTestEnvironment& operator =(
        const TimeServiceTestEnvironment&) = delete;
      void locked_set(
        boost::posix_time::ptime time, boost::unique_lock<Mutex>& lock);
      void add(TestTimeClient* client);
      void remove(TestTimeClient* client);
      void add(TestTimer* timer);
      void remove(TestTimer* timer);
  };

  inline TimeServiceTestEnvironment::TimeServiceTestEnvironment()
    : TimeServiceTestEnvironment(
        boost::posix_time::second_clock::universal_time()) {}

  inline TimeServiceTestEnvironment::TimeServiceTestEnvironment(
      boost::posix_time::ptime time)
      : m_next_trigger(boost::posix_time::pos_infin) {
    try {
      set(time);
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  inline TimeServiceTestEnvironment::~TimeServiceTestEnvironment() {
    close();
  }

  inline void TimeServiceTestEnvironment::set(boost::posix_time::ptime time) {
    if(time.is_special()) {
      boost::throw_with_location(
        TimeServiceTestEnvironmentException("Invalid date/time."));
    }
    flush_pending_routines();
    auto lock = boost::unique_lock(m_mutex);
    locked_set(time, lock);
  }

  inline void TimeServiceTestEnvironment::advance(
      boost::posix_time::time_duration duration) {
    flush_pending_routines();
    auto lock = boost::unique_lock(m_mutex);
    if(m_current_time == boost::posix_time::not_a_date_time) {
      m_current_time = boost::posix_time::ptime(
        boost::gregorian::date(2016, 8, 14), boost::posix_time::seconds(0));
    }
    locked_set(m_current_time + duration, lock);
  }

  inline boost::posix_time::ptime TimeServiceTestEnvironment::get_time() const {
    auto lock = boost::unique_lock(m_mutex);
    return m_current_time;
  }

  inline void TimeServiceTestEnvironment::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    auto pending_timers = m_timers.load();
    for(auto& pending_timer : pending_timers) {
      pending_timer.m_timer->fail();;
    }
    m_open_state.close();
    flush_pending_routines();
  }

  inline void TimeServiceTestEnvironment::locked_set(
      boost::posix_time::ptime time, boost::unique_lock<Mutex>& lock) {
    if(m_current_time != boost::posix_time::not_a_date_time &&
        m_current_time >= time) {
      return;
    }
    auto delta = [&] {
      if(m_current_time == boost::posix_time::not_a_date_time) {
        return boost::posix_time::time_duration();
      } else {
        return time - m_current_time;
      }
    }();
    while(delta > m_next_trigger) {
      delta -= m_next_trigger;
      locked_set(m_current_time + m_next_trigger, lock);
    }
    m_next_trigger -= delta;
    if(m_next_trigger <= boost::posix_time::seconds(0)) {
      m_next_trigger = boost::posix_time::pos_infin;
    }
    m_current_time = time;
    m_time_clients.for_each([&] (auto& client) {
      client->set(time);
    });
    auto expired_timers = std::vector<TestTimer*>();
    m_timers.erase_if([&] (auto& timer) {
      timer.m_time_remaining -= delta;
      if(timer.m_time_remaining <= boost::posix_time::seconds(0)) {
        expired_timers.push_back(timer.m_timer);
        return true;
      } else {
        m_next_trigger = std::min(m_next_trigger, timer.m_time_remaining);
        return false;
      }
    });
    {
      auto releaser = release(lock);
      for(auto& expired_timer : expired_timers) {
        expired_timer->trigger();
      }
      flush_pending_routines();
    }
  }

  inline void TimeServiceTestEnvironment::add(TestTimeClient* time_client) {
    m_time_clients.with([&] (auto& time_clients) {
      time_clients.push_back(time_client);
      if(m_current_time != boost::posix_time::not_a_date_time) {
        time_client->set(m_current_time);
      }
    });
  }

  inline void TimeServiceTestEnvironment::remove(TestTimeClient* client) {
    m_time_clients.erase(client);
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

  inline void TimeServiceTestEnvironment::remove(TestTimer* timer) {
    m_timers.erase_if([&] (auto& entry) {
      return entry.m_timer == timer;
    });
  }

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

  inline void TestTimer::trigger() {
    {
      auto lock = boost::lock_guard(m_mutex);
      m_has_started = false;
    }
    m_timer.trigger();
  }

  inline void TestTimer::fail() {
    {
      auto lock = boost::lock_guard(m_mutex);
      m_has_started = false;
    }
    m_timer.fail();
  }

  inline TestTimeClient::TestTimeClient(
      Ref<TimeServiceTestEnvironment> environment)
      : m_environment(environment.get()) {
    try {
      m_environment->add(this);
    } catch(const std::exception&) {
      close();
      throw;
    }
  }

  inline TestTimeClient::~TestTimeClient() {
    close();
  }

  inline boost::posix_time::ptime TestTimeClient::get_time() {
    return m_time_client.get_time();
  }

  inline void TestTimeClient::close() {
    if(m_open_state.set_closing()) {
      return;
    }
    m_time_client.close();
    m_environment->remove(this);
    m_open_state.close();
  }

  inline void TestTimeClient::set(boost::posix_time::ptime time) {
    m_time_client.set(time);
  }
}

#endif
