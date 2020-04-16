#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"
#include "Beam/Serialization/ShuttleArray.hpp"
#include "Beam/Serialization/TypeRegistry.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Serialization;
using namespace Beam::Serialization::Tests;

namespace {
  struct BinaryTest {
    using SenderType = BinarySender<SharedBuffer>;
    using ReceiverType = BinaryReceiver<SharedBuffer>;

    static auto MakeSender() {
      return BinarySender<SharedBuffer>();
    }

    static auto MakeSender(
        Ref<TypeRegistry<BinarySender<SharedBuffer>>> registry) {
      return BinarySender<SharedBuffer>(Ref(registry));
    }

    static auto MakeReceiver() {
      return BinaryReceiver<SharedBuffer>();
    }

    static auto MakeReceiver(
        Ref<TypeRegistry<BinarySender<SharedBuffer>>> registry) {
      return BinaryReceiver<SharedBuffer>(Ref(registry));
    }
  };

  struct JsonTest {
    using SenderType = JsonSender<SharedBuffer>;
    using ReceiverType = JsonReceiver<SharedBuffer>;

    static auto MakeSender() {
      return JsonSender<SharedBuffer>();
    }

    static auto MakeSender(
        Ref<TypeRegistry<JsonSender<SharedBuffer>>> registry) {
      return JsonSender<SharedBuffer>(Ref(registry));
    }

    static auto MakeReceiver() {
      return JsonReceiver<SharedBuffer>();
    }

    static auto MakeReceiver(
        Ref<TypeRegistry<JsonSender<SharedBuffer>>> registry) {
      return JsonReceiver<SharedBuffer>(Ref(registry));
    }
  };
}

TEST_SUITE("DataShuttle") {
  TEST_CASE_TEMPLATE("shuttle", T, BinaryTest, JsonTest) {
    SUBCASE("bool") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), true);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), true);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), false);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), false);
    }

    SUBCASE("char") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), '\0');
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), '\0');
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 'a');
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 'a');
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(),
        static_cast<signed char>(-1));
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(),
        static_cast<signed char>(-1));
    }

    SUBCASE("unsigned_char") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), '\0');
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), '\0');
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 'a');
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 'a');
    }

    SUBCASE("short") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(),
        static_cast<short>(0));
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(),
        static_cast<short>(0));
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(),
        static_cast<short>(1));
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(),
        static_cast<short>(1));
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(),
        static_cast<short>(-1));
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(),
        static_cast<short>(-1));
    }

    SUBCASE("unsigned_short") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(),
        static_cast<unsigned short>(0));
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(),
        static_cast<unsigned short>(0));
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(),
        static_cast<unsigned short>(1));
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(),
        static_cast<unsigned short>(1));
    }

    SUBCASE("int") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 0);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 0);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 1);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 1);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), -1);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), -1);
    }

    SUBCASE("unsigned_int") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 0U);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 0U);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 1U);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 1U);
    }

    SUBCASE("long") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 0L);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 0L);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 1L);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 1L);
    }

    SUBCASE("unsigned_long") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 0UL);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 0UL);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 1UL);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 1UL);
    }

    SUBCASE("long_long") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 0LL);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 0LL);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 1LL);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 1LL);
    }

    SUBCASE("unsigned_long_long") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 0ULL);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 0ULL);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 1ULL);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 1ULL);
    }

    SUBCASE("float") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 0.0F);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 0.0F);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 1.0F);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 1.0F);
    }

    SUBCASE("ShuttlingDouble") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 0.0);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 0.0);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), 1.0);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), 1.0);
    }

    SUBCASE("string") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(),
        std::string("hello world"));
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), std::string(""));
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(),
        std::string("hello world"));
    }

    SUBCASE("sequence") {
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(),
        std::array<int, 5>{5, 4, 3, 2, 1});
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(),
        std::array<int, 3>{1, 2, 3});
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(),
        std::array<int, 4>{4, 1, 2, 3});
    }

    SUBCASE("struct_with_free_shuttle") {
      auto object = StructWithFreeShuttle{'1', 2, 3.3};
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), object);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), object);
    }

    SUBCASE("class_shuttle_method") {
      auto object = ClassWithShuttleMethod('1', 2, 3.3);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), object);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), object);
    }

    SUBCASE("class_send_receive_methods") {
      auto object = ClassWithSendReceiveMethods('1', 2, 3.3);
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), object);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), object);
    }

    SUBCASE("class_versioning") {
      auto outValue = ClassWithVersioning(1, 2, 3);
      auto sender = T::MakeSender();
      auto buffer = typename T::SenderType::Sink();
      auto inValue = ClassWithVersioning();
      auto receiver = T::MakeReceiver();

      // Shuttle version 0.
      sender.SetSink(Ref(buffer));
      sender.Send(outValue, 0);
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(inValue);
      REQUIRE(inValue == ClassWithVersioning(1, 0, 0));
      buffer.Reset();

      // Shuttle version 1.
      sender.SetSink(Ref(buffer));
      sender.Send(outValue, 1);
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(inValue);
      REQUIRE(inValue == ClassWithVersioning(1, 2, 0));
      buffer.Reset();

      // Shuttle version 2.
      sender.SetSink(Ref(buffer));
      sender.Send(outValue, 2);
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(inValue);
      REQUIRE(inValue == ClassWithVersioning(1, 2, 3));
    }

    SUBCASE("null_pointer") {
      auto outValue = (PolymorphicDerivedClassA*)(nullptr);
      auto typeRegistry = TypeRegistry<typename T::SenderType>();
      typeRegistry.template Register<PolymorphicDerivedClassA>(
        "PolymorphicDerivedClassA");
      auto sender = T::MakeSender(Ref(typeRegistry));
      auto receiver = T::MakeReceiver(Ref(typeRegistry));
      auto buffer = typename T::SenderType::Sink();
      sender.SetSink(Ref(buffer));
      sender.Send(outValue);
      auto inValue = (PolymorphicBaseClass*)(nullptr);
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(inValue);
      REQUIRE(inValue == nullptr);
    }

    SUBCASE("polymorphic_class") {
      auto outValue = std::make_unique<PolymorphicDerivedClassA>();
      auto typeRegistry = TypeRegistry<typename T::SenderType>();
      typeRegistry.template Register<PolymorphicDerivedClassA>(
        "PolymorphicDerivedClassA");
      typeRegistry.template Register<PolymorphicDerivedClassB>(
        "PolymorphicDerivedClassB");
      auto sender = T::MakeSender(Ref(typeRegistry));
      auto receiver = T::MakeReceiver(Ref(typeRegistry));
      auto buffer = typename T::SenderType::Sink();
      sender.SetSink(Ref(buffer));
      sender.Send(outValue.get());
      auto inValue = (PolymorphicBaseClass*)(nullptr);
      receiver.SetSource(Ref(buffer));
      receiver.Shuttle(inValue);
      REQUIRE(inValue->ToString() == outValue->ToString());
      delete inValue;
    }

    SUBCASE("proxy_functions") {
      auto object = ProxiedFunctionType("hello world");
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), object);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), object);
    }

    SUBCASE("proxy_methods") {
      auto object = ProxiedMethodType("hello world");
      TestShuttlingReference(T::MakeSender(), T::MakeReceiver(), object);
      TestShuttlingConstant(T::MakeSender(), T::MakeReceiver(), object);
    }
  }
}
