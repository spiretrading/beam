#ifndef BEAM_JSONSHUTTLETESTER_HPP
#define BEAM_JSONSHUTTLETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/SerializationTests/DataShuttleTester.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Serialization {
namespace Tests {

  /*! \class JsonShuttleTester
      \brief Tests shuttling data using the JSON format.
   */
  class JsonShuttleTester : public DataShuttleTester<
      JsonSender<IO::SharedBuffer>, JsonReceiver<IO::SharedBuffer>> {
    protected:
      virtual JsonSender<IO::SharedBuffer> MakeSender();

      virtual JsonSender<IO::SharedBuffer> MakeSender(
        RefType<TypeRegistry<JsonSender<IO::SharedBuffer>>> registry);

      virtual JsonReceiver<IO::SharedBuffer> MakeReceiver();

      virtual JsonReceiver<IO::SharedBuffer> MakeReceiver(
        RefType<TypeRegistry<JsonSender<IO::SharedBuffer>>> registry);

    private:
      typedef DataShuttleTester<JsonSender<IO::SharedBuffer>,
        JsonReceiver<IO::SharedBuffer>> Parent;
      CPPUNIT_TEST_SUB_SUITE(JsonShuttleTester, Parent);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
