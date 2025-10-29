#ifndef BEAM_SHUTTLE_TEST_SUITE_HPP
#define BEAM_SHUTTLE_TEST_SUITE_HPP
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleArray.hpp"
#include "Beam/SerializationTests/ShuttleTestTypes.hpp"

namespace Beam::Tests {

  /**
   * Tests that a value can be sent and received using a DataShuttle.
   * @tparam S The type of Sender to use.
   * @tparam T The type of value to shuttle.
   * @param value The value to shuttle.
   */
  template<typename S, typename T>
  void test_roundtrip(const T& value) {
    using Sender = S;
    auto sender = Sender();
    auto buffer = typename Sender::Sink();
    sender.set(Ref(buffer));
    sender.shuttle(value);
    using Receiver = inverse_t<Sender>;
    auto receiver = Receiver();
    receiver.set(Ref(buffer));
    auto received_value = T();
    receiver.shuttle(received_value);
    REQUIRE(value == received_value);
  }

  TEST_CASE_TEMPLATE_DEFINE("Shuttle", T, ShuttleTestSuite) {
    SUBCASE("bool") {
      test_roundtrip<T>(true);
      test_roundtrip<T>(false);
    }

    SUBCASE("char") {
      test_roundtrip<T>('\0');
      test_roundtrip<T>('a');
      test_roundtrip<T>(static_cast<signed char>(-1));
    }

    SUBCASE("unsigned_char") {
      test_roundtrip<T>(static_cast<unsigned char>('\0'));
      test_roundtrip<T>(static_cast<unsigned char>('a'));
    }

    SUBCASE("short") {
      test_roundtrip<T>(static_cast<short>(0));
      test_roundtrip<T>(static_cast<short>(1));
      test_roundtrip<T>(static_cast<short>(-1));
    }

    SUBCASE("unsigned_short") {
      test_roundtrip<T>(static_cast<unsigned short>(0));
      test_roundtrip<T>(static_cast<unsigned short>(1));
    }

    SUBCASE("int") {
      test_roundtrip<T>(0);
      test_roundtrip<T>(1);
      test_roundtrip<T>(-1);
    }

    SUBCASE("unsigned_int") {
      test_roundtrip<T>(0U);
      test_roundtrip<T>(1U);
    }

    SUBCASE("long") {
      test_roundtrip<T>(0L);
      test_roundtrip<T>(1L);
    }

    SUBCASE("unsigned_long") {
      test_roundtrip<T>(0UL);
      test_roundtrip<T>(1UL);
    }

    SUBCASE("long_long") {
      test_roundtrip<T>(0LL);
      test_roundtrip<T>(1LL);
    }

    SUBCASE("unsigned_long_long") {
      test_roundtrip<T>(0ULL);
      test_roundtrip<T>(1ULL);
    }

    SUBCASE("float") {
      test_roundtrip<T>(0.0F);
      test_roundtrip<T>(1.0F);
    }

    SUBCASE("double") {
      test_roundtrip<T>(0.0);
      test_roundtrip<T>(1.0);
    }

    SUBCASE("string") {
      test_roundtrip<T>(std::string("hello world"));
      test_roundtrip<T>(std::string(""));
    }

    SUBCASE("buffer") {
      test_roundtrip<T>(from<SharedBuffer>("the quick brown fox"));
      test_roundtrip<T>(from<SharedBuffer>(""));
      test_roundtrip<T>(from<SharedBuffer>("hello world"));
    }

    SUBCASE("sequence") {
      test_roundtrip<T>(std::array<int, 0>());
      test_roundtrip<T>(std::array{1.1, 2.2, 3.3, 4.4});
      test_roundtrip<T>(std::array{5, 4, 3, 2, 1});
    }

    SUBCASE("StructWithFreeShuttle") {
      test_roundtrip<T>(StructWithFreeShuttle());
      test_roundtrip<T>(StructWithFreeShuttle('a', 2, 3.14));
    }

    SUBCASE("ClassWithShuttleMethod") {
      test_roundtrip<T>(ClassWithShuttleMethod());
      test_roundtrip<T>(ClassWithShuttleMethod('b', 3, 2.71));
    }

    SUBCASE("ClassWithSendReceiveMethods") {
      test_roundtrip<T>(ClassWithSendReceiveMethods());
      test_roundtrip<T>(ClassWithSendReceiveMethods('c', 4, 5.22));
    }

    SUBCASE("ClassWithVersioning") {
      auto out_value = ClassWithVersioning(1, 2, 3);
      auto sender = T();
      auto buffer = typename T::Sink();
      auto in_value = ClassWithVersioning();
      auto receiver = inverse_t<T>();

      SUBCASE("version0") {
        sender.set(Ref(buffer));
        sender.send_version(out_value, 0);
        receiver.set(Ref(buffer));
        receiver.shuttle(in_value);
        REQUIRE(in_value == ClassWithVersioning(1, 0, 0));
      }

      SUBCASE("version1") {
        sender.set(Ref(buffer));
        sender.send_version(out_value, 1);
        receiver.set(Ref(buffer));
        receiver.shuttle(in_value);
        REQUIRE(in_value == ClassWithVersioning(1, 2, 0));
      }

      SUBCASE("version2") {
        sender.set(Ref(buffer));
        sender.send_version(out_value, 2);
        receiver.set(Ref(buffer));
        receiver.shuttle(in_value);
        REQUIRE(in_value == ClassWithVersioning(1, 2, 3));
      }
    }

    SUBCASE("null") {
      auto out_value = static_cast<PolymorphicDerivedClassA*>(nullptr);
      auto registry = TypeRegistry<T>();
      registry.template add<PolymorphicDerivedClassA>(
        "PolymorphicDerivedClassA");
      auto sender = T(Ref(registry));
      auto receiver = inverse_t<T>(Ref(registry));
      auto buffer = typename T::Sink();
      sender.set(Ref(buffer));
      sender.send(out_value);
      auto in_value = static_cast<PolymorphicBaseClass*>(nullptr);
      receiver.set(Ref(buffer));
      receiver.shuttle(in_value);
      REQUIRE(!in_value);
    }

    SUBCASE("polymorphic_class") {
      auto out_value = std::make_unique<PolymorphicDerivedClassA>();
      auto registry = TypeRegistry<T>();
      registry.template add<PolymorphicDerivedClassA>(
        "PolymorphicDerivedClassA");
      registry.template add<PolymorphicDerivedClassB>(
        "PolymorphicDerivedClassB");
      auto sender = T(Ref(registry));
      auto receiver = inverse_t<T>(Ref(registry));
      auto buffer = typename T::Sink();
      sender.set(Ref(buffer));
      sender.send(out_value.get());
      auto in_value = static_cast<PolymorphicBaseClass*>(nullptr);
      receiver.set(Ref(buffer));
      receiver.shuttle(in_value);
      REQUIRE(in_value->to_string() == out_value->to_string());
      delete in_value;
    }

    SUBCASE("proxy_functions") {
      test_roundtrip<T>(ProxiedFunctionType("hello world"));
    }

    SUBCASE("proxy_methods") {
      test_roundtrip<T>(ProxiedMethodType("hello world"));
    }
  }
}

#endif
