#ifndef BEAM_REGISTRYSERVLETTESTER_HPP
#define BEAM_REGISTRYSERVLETTESTER_HPP
#include <boost/optional/optional.hpp>
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/ServiceLocatorTests/ServiceLocatorTestEnvironment.hpp"
#include "Beam/RegistryService/LocalRegistryDataStore.hpp"
#include "Beam/RegistryService/RegistryServlet.hpp"
#include "Beam/RegistryServiceTests/RegistryServiceTests.hpp"
#include "Beam/ServicesTests/ServicesTests.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace RegistryService {
namespace Tests {

  /*! \class RegistryServletTester
      \brief Tests the RegistryServlet class.
   */
  class RegistryServletTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceProtocolServer.
      using ServletContainer =
        Services::Tests::TestAuthenticatedServiceProtocolServletContainer<
        MetaRegistryServlet<std::shared_ptr<LocalRegistryDataStore>>>;

      virtual void setUp();

      virtual void tearDown();

      //! Test making a directory RegistryEntry.
      void TestMakeDirectory();

      //! Test making a registry value.
      void TestMakeValue();

      //! Test loading a path.
      void TestLoadPath();

      //! Test loading a registry value.
      void TestLoadValue();

    private:
      boost::optional<ServiceLocator::Tests::ServiceLocatorTestEnvironment>
        m_serviceLocatorEnvironment;
      std::shared_ptr<LocalRegistryDataStore> m_dataStore;
      boost::optional<ServletContainer> m_container;
      boost::optional<Services::Tests::TestServiceProtocolClient>
        m_clientProtocol;

      CPPUNIT_TEST_SUITE(RegistryServletTester);
        CPPUNIT_TEST(TestMakeDirectory);
        CPPUNIT_TEST(TestMakeValue);
        CPPUNIT_TEST(TestLoadPath);
        CPPUNIT_TEST(TestLoadValue);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
