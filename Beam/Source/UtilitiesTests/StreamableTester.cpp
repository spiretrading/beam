#include <doctest/doctest.h>
#include "Beam/Utilities/Streamable.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;

namespace {
  class TestStreamable : public Streamable {
    public:
      TestStreamable(std::string name)
        : m_name(std::move(name)) {}

    protected:
      std::ostream& to_stream(std::ostream& out) const override {
        return out << "TestStreamable(" << m_name << ')';
      }

    private:
      std::string m_name;
  };
}

TEST_SUITE("Streamable") {
  TEST_CASE("streamable_to_stream") {
    auto object = TestStreamable("test");
    REQUIRE(to_string(object) == "TestStreamable(test)");
  }

  TEST_CASE("streamable_to_string") {
    auto object = TestStreamable("example");
    auto result = to_string(object);
    REQUIRE(result == "TestStreamable(example)");
  }

  TEST_CASE("streamable_default_implementation") {
    auto object = Streamable();
    auto result = to_string(object);
    REQUIRE(result.find("Streamable") != std::string::npos);
  }

  TEST_CASE("stream_vector") {
    SUBCASE("empty_vector") {
      auto vector = std::vector<int>();
      REQUIRE(to_string(Stream(vector)) == "[]");
    }

    SUBCASE("single_element") {
      auto vector = std::vector<int>{42};
      REQUIRE(to_string(Stream(vector)) == "[42]");
    }

    SUBCASE("multiple_elements") {
      auto vector = std::vector<int>{1, 2, 3, 4, 5};
      REQUIRE(to_string(Stream(vector)) == "[1, 2, 3, 4, 5]");
    }

    SUBCASE("string_vector") {
      auto vector = std::vector<std::string>{"hello", "world"};
      REQUIRE(to_string(Stream(vector)) == "[hello, world]");
    }
  }

  TEST_CASE("stream_map") {
    SUBCASE("empty_map") {
      auto map = std::map<int, std::string>();
      REQUIRE(to_string(Stream(map)) == "{}");
    }

    SUBCASE("single_entry") {
      auto map = std::map<int, std::string>{{1, "one"}};
      REQUIRE(to_string(Stream(map)) == "{1: one}");
    }

    SUBCASE("multiple_entries") {
      auto map =
        std::map<int, std::string>{{1, "one"}, {2, "two"}, {3, "three"}};
      REQUIRE(to_string(Stream(map)) == "{1: one, 2: two, 3: three}");
    }

    SUBCASE("string_keys") {
      auto map = std::map<std::string, int>{{"a", 1}, {"b", 2}};
      REQUIRE(to_string(Stream(map)) == "{a: 1, b: 2}");
    }
  }

  TEST_CASE("stream_set") {
    SUBCASE("empty_set") {
      auto set = std::set<int>();
      REQUIRE(to_string(Stream(set)) == "{}");
    }

    SUBCASE("single_element") {
      auto set = std::set<int>{42};
      REQUIRE(to_string(Stream(set)) == "{42}");
    }

    SUBCASE("multiple_elements") {
      auto set = std::set<int>{1, 2, 3, 4, 5};
      REQUIRE(to_string(Stream(set)) == "{1, 2, 3, 4, 5}");
    }

    SUBCASE("string_set") {
      auto set = std::set<std::string>{"apple", "banana", "cherry"};
      REQUIRE(to_string(Stream(set)) == "{apple, banana, cherry}");
    }
  }

  TEST_CASE("stream_nested_containers") {
    SUBCASE("vector_of_vectors") {
      auto nested = std::vector<std::vector<int>>{{1, 2}, {3, 4}, {5}};
      REQUIRE(to_string(Stream(nested)) == "[[1, 2], [3, 4], [5]]");
    }

    SUBCASE("map_of_vectors") {
      auto map = std::map<int, std::vector<int>>{{1, {10, 20}}, {2, {30}}};
      REQUIRE(to_string(Stream(map)) == "{1: [10, 20], 2: [30]}");
    }
  }
}
