#ifndef BEAM_BINARYSHUTTLETESTER_HPP
#define BEAM_BINARYSHUTTLETESTER_HPP
#include <cppunit/extensions/HelperMacros.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/SerializationTests/DataShuttleTester.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace Serialization {
namespace Tests {

  /*! \class BinaryShuttleTester
      \brief Tests shuttling data using the binary format.
   */
  class BinaryShuttleTester : public DataShuttleTester<
      BinarySender<IO::SharedBuffer>, BinaryReceiver<IO::SharedBuffer>> {
    protected:
      virtual BinarySender<IO::SharedBuffer> MakeSender();

      virtual BinarySender<IO::SharedBuffer> MakeSender(
        Ref<TypeRegistry<BinarySender<IO::SharedBuffer>>> registry);

      virtual BinaryReceiver<IO::SharedBuffer> MakeReceiver();

      virtual BinaryReceiver<IO::SharedBuffer> MakeReceiver(
        Ref<TypeRegistry<BinarySender<IO::SharedBuffer>>> registry);

    private:
      typedef DataShuttleTester<BinarySender<IO::SharedBuffer>,
        BinaryReceiver<IO::SharedBuffer>> Parent;
      CPPUNIT_TEST_SUB_SUITE(BinaryShuttleTester, Parent);
      BEAM_CPPUNIT_TEST_SUITE_END();
  };
}
}
}

#endif
