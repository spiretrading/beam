#ifndef BEAM_SERVICELOCATORCLIENTTESTER_HPP
#define BEAM_SERVICELOCATORCLIENTTESTER_HPP
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ServiceLocator/ServiceLocatorClient.hpp"
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace ServiceLocator {
namespace Tests {

  /*! \class ServiceLocatorClientTester
      \brief Tests the ServiceLocatorClient class.
   */
  class ServiceLocatorClientTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceLocatorClient.
      using TestServiceLocatorClient = ServiceLocatorClient<
        Services::Tests::TestServiceProtocolClientBuilder>;

      virtual void setUp();

      virtual void tearDown();

      //! Test logging in and getting accepted.
      void TestLoginAccepted();

      //! Test logging in and getting rejected.
      void TestLoginRejected();

      //! Tests monitoring a DirectoryEntry.
      void TestMonitorDirectoryEntry();

    private:
      boost::optional<Services::Tests::TestServiceProtocolServer>
        m_protocolServer;
      boost::optional<TestServiceLocatorClient> m_serviceClient;

      CPPUNIT_TEST_SUITE(ServiceLocatorClientTester);
        CPPUNIT_TEST(TestLoginAccepted);
        CPPUNIT_TEST(TestLoginRejected);
        CPPUNIT_TEST(TestMonitorDirectoryEntry);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
