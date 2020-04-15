#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/NativeValue.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("NativeValue") {
  TEST_CASE("make_native_value") {
    auto intValue = NativeValue(5);
    REQUIRE(intValue.GetType()->GetNativeType() == typeid(int));
    auto stringValue = NativeValue(std::string("hello world"));
    REQUIRE(stringValue.GetType()->GetNativeType() == typeid(std::string));
  }

  TEST_CASE("int") {
    auto nativeValue = NativeValue(123);
    REQUIRE(nativeValue.GetType() == NativeDataType<int>());
    REQUIRE(nativeValue.GetValue<int>() == 123);
    auto defaultValue = NativeValue(int());
    REQUIRE(defaultValue.GetType() == NativeDataType<int>());
    REQUIRE(defaultValue.GetValue<int>() == 0);
  }

  TEST_CASE("decimal") {
    auto nativeValue = NativeValue(3.14);
    REQUIRE(nativeValue.GetType() == NativeDataType<double>());
    REQUIRE(nativeValue.GetValue<double>() == 3.14);
    auto defaultValue = NativeValue(double());
    REQUIRE(defaultValue.GetType() == NativeDataType<double>());
    REQUIRE(defaultValue.GetValue<double>() == 0);
  }

  TEST_CASE("string") {
    auto nativeValue = NativeValue(std::string("hello world"));
    REQUIRE(nativeValue.GetType() == NativeDataType<std::string>());
    REQUIRE(nativeValue.GetValue<std::string>() == "hello world");
    auto defaultValue = NativeValue(std::string());
    REQUIRE(defaultValue.GetType() == NativeDataType<std::string>());
    REQUIRE(defaultValue.GetValue<std::string>().empty());
  }
}
