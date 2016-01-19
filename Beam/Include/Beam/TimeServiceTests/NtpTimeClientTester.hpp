#ifndef BEAM_NTPTIMECLIENTTESTER_HPP
#define BEAM_NTPTIMECLIENTTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/TimeService/NtpTimeClient.hpp"
#include "Beam/TimeServiceTests/TimeServiceTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace TimeService {
namespace Tests {

  /*! \class NtpTimeClientTester
      \brief Tests the NtpTimeClient class.
   */
  class NtpTimeClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The server accepting NtpTimeClients.
      typedef IO::LocalServerConnection<IO::SharedBuffer> ServerConnection;

      //! The NtpTimeClient to test.
      typedef NtpTimeClient<IO::LocalClientChannel<IO::SharedBuffer>,
        Threading::TriggerTimer*> TestTimeClient;

      virtual void setUp();

      virtual void tearDown();

      //! Tests converting to/from NtpTimestamp and ptimes.
      void TestNtpTimestampConversions();

      //! Tests opening the client.
      void TestOpen();

    private:
      CPPUNIT_TEST_SUITE(NtpTimeClientTester);
        CPPUNIT_TEST(TestNtpTimestampConversions);
        CPPUNIT_TEST(TestOpen);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
