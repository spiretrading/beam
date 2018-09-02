#include "Beam/SerializationTests/JsonShuttleTester.hpp"
#include "Beam/Serialization/JsonSender.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;
using namespace boost;

JsonSender<SharedBuffer> JsonShuttleTester::MakeSender() {
  return {};
}

JsonSender<SharedBuffer> JsonShuttleTester::MakeSender(
    Ref<TypeRegistry<JsonSender<SharedBuffer>>> registry) {
  return {Ref(registry)};
}

JsonReceiver<SharedBuffer> JsonShuttleTester::MakeReceiver() {
  return {};
}

JsonReceiver<SharedBuffer> JsonShuttleTester::MakeReceiver(
    Ref<TypeRegistry<JsonSender<SharedBuffer>>> registry) {
  return {Ref(registry)};
}
