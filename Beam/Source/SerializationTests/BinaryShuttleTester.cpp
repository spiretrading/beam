#include "Beam/SerializationTests/BinaryShuttleTester.hpp"
#include "Beam/Serialization/BinarySender.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
using namespace boost;

BinarySender<SharedBuffer> BinaryShuttleTester::MakeSender() {
  return BinarySender<SharedBuffer>();
}

BinarySender<SharedBuffer> BinaryShuttleTester::MakeSender(
    Ref<TypeRegistry<BinarySender<SharedBuffer>>> registry) {
  return BinarySender<SharedBuffer>(Ref(registry));
}

BinaryReceiver<SharedBuffer> BinaryShuttleTester::MakeReceiver() {
  return BinaryReceiver<SharedBuffer>();
}

BinaryReceiver<SharedBuffer> BinaryShuttleTester::MakeReceiver(
    Ref<TypeRegistry<BinarySender<SharedBuffer>>> registry) {
  return BinaryReceiver<SharedBuffer>(Ref(registry));
}
