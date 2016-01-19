#ifndef BEAM_QUERIESEXPRESSIONSSUBSCRIPTIONSTESTER_HPP
#define BEAM_QUERIESEXPRESSIONSSUBSCRIPTIONSTESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/Codecs/NullDecoder.hpp"
#include "Beam/Codecs/NullEncoder.hpp"
#include "Beam/IO/NullChannel.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Queries/BasicQuery.hpp"
#include "Beam/Queries/ExpressionSubscriptions.hpp"
#include "Beam/Queries/SequencedValue.hpp"
#include "Beam/QueriesTests/QueriesTests.hpp"
#include "Beam/Services/ServiceProtocolClient.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Threading/TriggerTimer.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Queries {
namespace Tests {

  /*  \class ExpressionSubscriptionsTester
      \brief Tests the ExpressionSubscriptions class.
   */
  class ExpressionSubscriptionsTester : public CPPUNIT_NS::TestFixture {
    public:

      //! The type of ServiceProtocolClient used for testing.
      using ServiceProtocolClient = Services::ServiceProtocolClient<
        Services::MessageProtocol<IO::NullChannel,
        Serialization::BinarySender<IO::SharedBuffer>, Codecs::NullEncoder>,
        Threading::TriggerTimer>;

      //! Tests publishing a value.
      void TestPublish();

    private:
      CPPUNIT_TEST_SUITE(ExpressionSubscriptionsTester);
        CPPUNIT_TEST(TestPublish);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
