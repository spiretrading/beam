#ifndef BEAM_UIDCLIENTTESTER_HPP
#define BEAM_UIDCLIENTTESTER_HPP
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/UidService/UidClient.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace UidService {
namespace Tests {

  /*! \class UidClientTester
      \brief Tests the UidClient class.
   */
  class UidClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of UidClient.
      using TestUidClient =
        UidClient<Services::Tests::TestServiceProtocolClientBuilder>;

      virtual void setUp();

      virtual void tearDown();

      //! Tests requesting a single UID.
      void TestSingleUidRequest();

      //! Tests requesting multiple UIDs one after another.
      void TestSequentialUidRequests();

      //! Tests requesting multiple UIDs before the server gets a chance to
      //! service the first request.
      void TestSimultaneousUidRequests();

      //! Tests requesting so many UIDs that it requires two server-side
      //! requests to satisfy.
      void TestMultipleServerRequests();

    private:
      boost::optional<Services::Tests::TestServiceProtocolServer>
        m_protocolServer;
      boost::optional<TestUidClient> m_uidClient;

      CPPUNIT_TEST_SUITE(UidClientTester);
        CPPUNIT_TEST(TestSingleUidRequest);
        CPPUNIT_TEST(TestSequentialUidRequests);
        CPPUNIT_TEST(TestSimultaneousUidRequests);
        CPPUNIT_TEST(TestMultipleServerRequests);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
