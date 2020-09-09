#ifndef BEAM_TEST_TIME_CLIENT_HPP
#define BEAM_TEST_TIME_CLIENT_HPP
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"
#include "Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp"
#include "Beam/TimeServiceTests/TimeServiceTests.hpp"

namespace Beam::TimeService::Tests {

  /** A TimeClient used by the TestEnvironment. */
  class TestTimeClient {
    public:

      /**
       * Constructs a TestTimeClient.
       * @param environment The TimeServiceTestEnvironment this client belongs
       *        to.
       */
      TestTimeClient(Ref<TimeServiceTestEnvironment> environment);

      ~TestTimeClient();

      boost::posix_time::ptime GetTime();

      void Close();

    private:
      friend class TimeServiceTestEnvironment;
      TimeServiceTestEnvironment* m_environment;
      TimeService::FixedTimeClient m_timeClient;
      IO::OpenState m_openState;

      TestTimeClient(const TestTimeClient&) = delete;
      TestTimeClient& operator =(const TestTimeClient&) = delete;
      void SetTime(boost::posix_time::ptime time);
  };

  inline TestTimeClient::TestTimeClient(
      Ref<TimeServiceTestEnvironment> environment)
      : m_environment(environment.Get()) {
    try {
      m_environment->Add(this);
    } catch(const std::exception&) {
      Close();
      BOOST_RETHROW;
    }
  }

  inline TestTimeClient::~TestTimeClient() {
    Close();
  }

  inline boost::posix_time::ptime TestTimeClient::GetTime() {
    return m_timeClient.GetTime();
  }

  inline void TestTimeClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    m_timeClient.Close();
    m_environment->Remove(this);
    m_openState.Close();
  }

  inline void TestTimeClient::SetTime(boost::posix_time::ptime time) {
    m_timeClient.SetTime(time);
  }

  inline void TimeServiceTestEnvironment::Add(TestTimeClient* timeClient) {
    m_timeClients.With([&] (auto& timeClients) {
      timeClients.push_back(timeClient);
      if(m_currentTime != boost::posix_time::not_a_date_time) {
        timeClient->SetTime(m_currentTime);
      }
    });
  }
}

#endif
