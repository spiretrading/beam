#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/NativeDataType.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("NativeDataType") {
  TEST_CASE("int") {
    auto nativeType = NativeDataType<int>();
    REQUIRE(nativeType.GetNativeType() == typeid(int));
    auto type = DataType(nativeType);
    REQUIRE(type->GetNativeType() == typeid(int));
    REQUIRE(type == nativeType);
    REQUIRE(type != NativeDataType<std::string>());
  }

  TEST_CASE("string") {
    auto nativeType = NativeDataType<std::string>();
    REQUIRE(nativeType.GetNativeType() == typeid(std::string));
    auto type = DataType(nativeType);
    REQUIRE(type->GetNativeType() == typeid(std::string));
    REQUIRE(type == nativeType);
    REQUIRE(type != NativeDataType<int>());
  }
}
