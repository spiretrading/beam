#ifndef BEAM_TESTTIMECLIENT_HPP
#define BEAM_TESTTIMECLIENT_HPP
#include <boost/noncopyable.hpp>
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"
#include "Beam/TimeServiceTests/TimeServiceTestEnvironment.hpp"
#include "Beam/TimeServiceTests/TimeServiceTests.hpp"

namespace Beam {
namespace TimeService {
namespace Tests {

  /*! \class TestTimeClient
      \brief A TimeClient used by the TestEnvironment.
   */
  class TestTimeClient : private boost::noncopyable {
    public:

      //! Constructs a TestTimeClient.
      /*!
        \param environment The TimeServiceTestEnvironment this client belongs
               to.
      */
      TestTimeClient(Ref<TimeServiceTestEnvironment> environment);

      ~TestTimeClient();

      boost::posix_time::ptime GetTime();

      void Open();

      void Close();

    private:
      friend class TimeServiceTestEnvironment;
      TimeServiceTestEnvironment* m_environment;
      TimeService::FixedTimeClient m_timeClient;
      IO::OpenState m_openState;

      void Shutdown();
      void SetTime(boost::posix_time::ptime time);
  };

  inline TestTimeClient::TestTimeClient(
      Ref<TimeServiceTestEnvironment> environment)
      : m_environment{environment.Get()} {}

  inline TestTimeClient::~TestTimeClient() {
    Close();
  }

  inline boost::posix_time::ptime TestTimeClient::GetTime() {
    return m_timeClient.GetTime();
  }

  inline void TestTimeClient::Open() {
    if(m_openState.SetOpening()) {
      return;
    }
    try {
      m_environment->Add(this);
      m_timeClient.Open();
    } catch(const std::exception&) {
      m_openState.SetOpenFailure();
      Shutdown();
    }
    m_openState.SetOpen();
  }

  inline void TestTimeClient::Close() {
    if(m_openState.SetClosing()) {
      return;
    }
    Shutdown();
  }

  inline void TestTimeClient::Shutdown() {
    m_timeClient.Close();
    m_environment->Remove(this);
    m_openState.SetClosed();
  }

  inline void TestTimeClient::SetTime(boost::posix_time::ptime time) {
    m_timeClient.SetTime(time);
  }

  inline void TimeServiceTestEnvironment::Add(TestTimeClient* timeClient) {
    m_timeClients.With(
      [&] (auto& timeClients) {
        timeClients.push_back(timeClient);
        if(m_currentTime != boost::posix_time::not_a_date_time) {
          timeClient->SetTime(m_currentTime);
        }
      });
  }
}
}
}

#endif
