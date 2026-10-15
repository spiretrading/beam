#include <doctest/doctest.h>
#include "Beam/Pointers/ClonePtr.hpp"
#include "Beam/Utilities/KeyValuePair.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

namespace {
  struct CopyCloner {
    template<typename T>
    T* operator ()(const T& value) const {
      return new T(value);
    }
  };

  struct Number {
    int m_value;
  };

  struct StreamValue {
    int m_value;
  };

  std::ostream& operator <<(std::ostream& out, const StreamValue& value) {
    return out << "stream:" << value.m_value;
  }
}

template<>
class std::formatter<Number> : public std::formatter<int> {
  public:
    auto format(const Number& value, auto& context) const {
      return std::formatter<int>::format(value.m_value, context);
    }
};

TEST_SUITE("ClonePtr") {
  TEST_CASE("format") {
    auto number = ClonePtr<Number, CopyCloner>(Number(42));
    REQUIRE(std::format("{}", number) == "42");
    REQUIRE(std::format("{:04x}", number) == "002a");
    REQUIRE(to_string(number) == "42");
    auto name = ClonePtr<std::string, CopyCloner>(std::string("name"));
    REQUIRE(std::format("{}", name) == "name");
    REQUIRE(std::format("{:*>8}", name) == "****name");
    REQUIRE(std::format("{:.2}", name) == "na");
    REQUIRE(std::format("{}", KeyValuePair(name, number)) == "(name 42)");
    REQUIRE_FALSE((std::formattable<ClonePtr<StreamValue, CopyCloner>, char>));
    auto streamed = ClonePtr<StreamValue, CopyCloner>(StreamValue(7));
    REQUIRE(to_string(streamed) == "stream:7");
  }
}
