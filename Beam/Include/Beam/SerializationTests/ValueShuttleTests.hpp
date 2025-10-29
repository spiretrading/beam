#ifndef BEAM_VALUE_SHUTTLE_TESTS_HPP
#define BEAM_VALUE_SHUTTLE_TESTS_HPP
#include <concepts>
#include <doctest/doctest.h>
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Serialization/BinaryReceiver.hpp"
#include "Beam/Serialization/BinarySender.hpp"
#include "Beam/Serialization/JsonReceiver.hpp"
#include "Beam/Serialization/JsonSender.hpp"

namespace Beam::Tests {

  /**
   * Tests that a value can be round-tripped through a shuttle.
   * @tparam T The type of the value.
   * @tparam F The type of the function to invoke with the received value.
   * @param value The value to shuttle.
   * @param f The function to invoke with the received value.
   */
  template<typename T, std::invocable<T&&> F>
  void test_round_trip_shuttle(const T& value, F&& f) {
    auto buffer = SharedBuffer();
    auto sender = BinarySender<SharedBuffer>();
    sender.set(Ref(buffer));
    sender.shuttle(value);
    auto receiver = BinaryReceiver<SharedBuffer>();
    receiver.set(Ref(buffer));
    auto received = SerializedValue<T>();
    received.initialize();
    receiver.shuttle(*received);
    f(std::move(*received));
  }

  /**
   * Tests that a value can be round-tripped through a shuttle.
   * @tparam T The type of the value.
   * @param value The value to shuttle.
   */
  template<std::equality_comparable T>
  void test_round_trip_shuttle(const T& value) {
    test_round_trip_shuttle(value, [&] (auto&& received) {
      REQUIRE(value == received);
    });
  }

  /**
   * Tests that a polymorphic value can be round-tripped through a shuttle.
   * @tparam T The type of the value.
   * @tparam F The type of the function used to register polymorphic types.
   * @param value The value to shuttle.
   * @param f The function used to register polymorphic types.
   */
  template<typename T,
    std::invocable<Out<TypeRegistry<BinarySender<SharedBuffer>>>> F,
    std::invocable<T&&> G>
  void test_polymorphic_round_trip_shuttle(const T& value, F&& f, G&& g) {
    auto buffer = SharedBuffer();
    auto type_registry = TypeRegistry<BinarySender<SharedBuffer>>();
    f(out(type_registry));
    auto sender = BinarySender<SharedBuffer>(Ref(type_registry));
    sender.set(Ref(buffer));
    sender.shuttle(value);
    auto receiver = BinaryReceiver<SharedBuffer>(Ref(type_registry));
    receiver.set(Ref(buffer));
    auto received = SerializedValue<T>();
    received.initialize();
    receiver.shuttle(*received);
    g(std::move(*received));
  }

  /**
   * Tests that a value can be round-tripped through a shuttle.
   * @tparam T The type of the value.
   * @param value The value to shuttle.
   */
  template<std::equality_comparable T,
    std::invocable<Out<TypeRegistry<BinarySender<SharedBuffer>>>> F>
  void test_polymorphic_round_trip_shuttle(const T& value, F&& f) {
    test_polymorphic_round_trip_shuttle(value, std::forward<F>(f),
      [&] (auto&& received) {
        REQUIRE(value == received);
      });
  }

  /**
   * Tests that two values serialize to the same JSON representation.
   * @tparam T The type of the values.
   * @param value The value to serialize.
   * @param expected The expected serialized value.
   */
  template<typename T>
  void test_json_equality(const T& value, const T& expected) {
    auto value_buffer = SharedBuffer();
    auto value_sender = JsonSender<SharedBuffer>();
    value_sender.set(Ref(value_buffer));
    value_sender.shuttle(value);
    auto expected_buffer = SharedBuffer();
    auto expected_sender = JsonSender<SharedBuffer>();
    expected_sender.set(Ref(expected_buffer));
    expected_sender.shuttle(expected);
    REQUIRE(std::string(value_buffer.get_data(), value_buffer.get_size()) ==
      std::string(expected_buffer.get_data(), expected_buffer.get_size()));
  }
}

#endif
