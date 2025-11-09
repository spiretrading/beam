#ifndef BEAM_TEST_TIME_CLIENT_HPP
#define BEAM_TEST_TIME_CLIENT_HPP
#include "Beam/IO/OpenState.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/TimeService/FixedTimeClient.hpp"

namespace Beam::Tests {
  class TimeServiceTestEnvironment;

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
}

#endif
