#ifndef BEAM_TEST_TIME_CLIENT_HPP
#define BEAM_TEST_TIME_CLIENT_HPP
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"
#include "Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp"

namespace Beam::Tests {

  /** A TimeClient used by the TestEnvironment. */
  class TestTimeClient {
    public:

      /**
       * Constructs a TestTimeClient.
       * @param environment The TimeServiceTestEnvironment this client belongs
       *        to.
       */
      explicit TestTimeClient(Ref<TimeServiceTestEnvironment> environment);

      ~TestTimeClient();

      boost::posix_time::ptime get_time();
      void close();

    private:
      friend class TimeServiceTestEnvironment;
      TimeServiceTestEnvironment* m_environment;
      FixedTimeClient m_time_client;
      OpenState m_open_state;

      TestTimeClient(const TestTimeClient&) = delete;
      TestTimeClient& operator =(const TestTimeClient&) = delete;
      void set(boost::posix_time::ptime time);
  };

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

  inline void TimeServiceTestEnvironment::add(TestTimeClient* time_client) {
    m_time_clients.with([&] (auto& time_clients) {
      time_clients.push_back(time_client);
      if(m_current_time != boost::posix_time::not_a_date_time) {
        time_client->set(m_current_time);
      }
    });
  }
}

#endif
