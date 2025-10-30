#include <string>
#include <unordered_map>
#include <doctest/doctest.h>
#include "Beam/SerializationTests/ValueShuttleTests.hpp"
#include "Beam/Utilities/FixedString.hpp"
#include "Beam/Utilities/ToString.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("FixedString") {
  TEST_CASE("default_construct") {
    auto str = FixedString<10>();
    REQUIRE(str.is_empty());
    REQUIRE(str.get_data()[0] == '\0');
  }

  TEST_CASE("construct_from_cstring") {
    auto str = FixedString<10>("hello");
    REQUIRE(!str.is_empty());
    REQUIRE(std::string(str.get_data()) == "hello");
  }

  TEST_CASE("construct_from_std_string") {
    auto source = std::string("world");
    auto str = FixedString<10>(source);
    REQUIRE(std::string(str.get_data()) == "world");
  }

  TEST_CASE("construct_from_string_view") {
    auto view = std::string_view("test");
    auto str = FixedString<10>(view);
    REQUIRE(std::string(str.get_data()) == "test");
  }

  TEST_CASE("construct_from_fixed_string") {
    auto str1 = FixedString<10>("original");
    auto str2 = FixedString<20>(str1);
    REQUIRE(std::string(str1.get_data()) == std::string(str2.get_data()));
  }

  TEST_CASE("truncate_long_string") {
    auto str = FixedString<5>("1234567890");
    REQUIRE(std::string(str.get_data()) == "12345");
  }

  TEST_CASE("assign_from_cstring") {
    auto str = FixedString<10>();
    str = "assigned";
    REQUIRE(std::string(str.get_data()) == "assigned");
  }

  TEST_CASE("assign_from_std_string") {
    auto str = FixedString<10>();
    str = std::string("assigned");
    REQUIRE(std::string(str.get_data()) == "assigned");
  }

  TEST_CASE("assign_from_string_view") {
    auto str = FixedString<10>();
    str = std::string_view("assigned");
    REQUIRE(std::string(str.get_data()) == "assigned");
  }

  TEST_CASE("assign_from_fixed_string") {
    auto str1 = FixedString<10>("source");
    auto str2 = FixedString<10>();
    str2 = str1;
    REQUIRE(std::string(str1.get_data()) == std::string(str2.get_data()));
  }

  TEST_CASE("assign_from_larger_fixed_string") {
    auto str1 = FixedString<20>("long_string");
    auto str2 = FixedString<5>();
    str2 = str1;
    REQUIRE(std::string(str2.get_data()) == "long_");
  }

  TEST_CASE("assign_from_smaller_fixed_string") {
    auto str1 = FixedString<5>("short");
    auto str2 = FixedString<20>();
    str2 = str1;
    REQUIRE(std::string(str2.get_data()) == "short");
  }

  TEST_CASE("reset") {
    auto str = FixedString<10>("hello");
    REQUIRE(!str.is_empty());
    str.reset();
    REQUIRE(str.is_empty());
    REQUIRE(str.get_data()[0] == '\0');
  }

  TEST_CASE("is_empty_on_empty_string") {
    auto str = FixedString<10>("");
    REQUIRE(str.is_empty());
  }

  TEST_CASE("is_empty_on_nonempty_string") {
    auto str = FixedString<10>("test");
    REQUIRE(!str.is_empty());
  }

  TEST_CASE("equality_same_size") {
    auto str1 = FixedString<10>("test");
    auto str2 = FixedString<10>("test");
    auto str3 = FixedString<10>("different");
    REQUIRE(str1 == str2);
    REQUIRE(str1 != str3);
  }

  TEST_CASE("equality_different_sizes") {
    auto str1 = FixedString<10>("test");
    auto str2 = FixedString<20>("test");
    auto str3 = FixedString<20>("different");
    REQUIRE(str1 == str2);
    REQUIRE(str1 != str3);
  }

  TEST_CASE("less_than_same_size") {
    auto str1 = FixedString<10>("abc");
    auto str2 = FixedString<10>("def");
    REQUIRE(str1 < str2);
    REQUIRE(!(str2 < str1));
  }

  TEST_CASE("less_than_different_sizes") {
    auto str1 = FixedString<10>("abc");
    auto str2 = FixedString<20>("def");
    REQUIRE(str1 < str2);
    REQUIRE(!(str2 < str1));
  }

  TEST_CASE("less_than_or_equal") {
    auto str1 = FixedString<10>("abc");
    auto str2 = FixedString<10>("abc");
    auto str3 = FixedString<10>("def");
    REQUIRE(str1 <= str2);
    REQUIRE(str1 <= str3);
    REQUIRE(!(str3 <= str1));
  }

  TEST_CASE("greater_than") {
    auto str1 = FixedString<10>("def");
    auto str2 = FixedString<10>("abc");
    REQUIRE(str1 > str2);
    REQUIRE(!(str2 > str1));
  }

  TEST_CASE("greater_than_or_equal") {
    auto str1 = FixedString<10>("def");
    auto str2 = FixedString<10>("def");
    auto str3 = FixedString<10>("abc");
    REQUIRE(str1 >= str2);
    REQUIRE(str1 >= str3);
    REQUIRE(!(str3 >= str1));
  }

  TEST_CASE("spaceship_operator") {
    auto str1 = FixedString<10>("abc");
    auto str2 = FixedString<10>("abc");
    auto str3 = FixedString<10>("def");
    REQUIRE((str1 <=> str2) == std::strong_ordering::equal);
    REQUIRE((str1 <=> str3) == std::strong_ordering::less);
    REQUIRE((str3 <=> str1) == std::strong_ordering::greater);
  }

  TEST_CASE("compare_with_string_view_equal") {
    auto str = FixedString<10>("test");
    REQUIRE(str == std::string_view("test"));
    REQUIRE(std::string_view("test") == str);
  }

  TEST_CASE("compare_with_string_view_not_equal") {
    auto str = FixedString<10>("test");
    REQUIRE(str != std::string_view("different"));
    REQUIRE(std::string_view("different") != str);
  }

  TEST_CASE("compare_with_string_view_less") {
    auto str = FixedString<10>("abc");
    REQUIRE(str < std::string_view("def"));
    REQUIRE(std::string_view("abc") < FixedString<10>("def"));
  }

  TEST_CASE("compare_with_string_view_greater") {
    auto str = FixedString<10>("def");
    REQUIRE(str > std::string_view("abc"));
    REQUIRE(std::string_view("def") > FixedString<10>("abc"));
  }

  TEST_CASE("hash_value_same_strings") {
    auto str1 = FixedString<10>("test");
    auto str2 = FixedString<20>("test");
    REQUIRE(hash_value(str1) == hash_value(str2));
  }

  TEST_CASE("hash_value_different_strings") {
    auto str1 = FixedString<10>("test1");
    auto str2 = FixedString<10>("test2");
    REQUIRE(hash_value(str1) != hash_value(str2));
  }

  TEST_CASE("std_hash") {
    auto str1 = FixedString<10>("test");
    auto str2 = FixedString<10>("test");
    auto hasher = std::hash<FixedString<10>>();
    REQUIRE(hasher(str1) == hasher(str2));
  }

  TEST_CASE("use_in_unordered_map") {
    auto map = std::unordered_map<FixedString<10>, int>();
    map[FixedString<10>("key1")] = 100;
    map[FixedString<10>("key2")] = 200;
    REQUIRE(map[FixedString<10>("key1")] == 100);
    REQUIRE(map[FixedString<10>("key2")] == 200);
  }

  TEST_CASE("empty_string_hash") {
    auto str = FixedString<10>("");
    REQUIRE(hash_value(str) == 0);
  }

  TEST_CASE("size_constant") {
    REQUIRE(FixedString<10>::SIZE == 10);
    REQUIRE(FixedString<100>::SIZE == 100);
  }

  TEST_CASE("null_termination") {
    auto str = FixedString<10>("hello");
    REQUIRE(str.get_data()[5] == '\0');
  }

  TEST_CASE("overwrite_with_shorter_string") {
    auto str = FixedString<10>("longstring");
    str = "short";
    REQUIRE(std::string(str.get_data()) == "short");
    REQUIRE(str.get_data()[5] == '\0');
  }

  TEST_CASE("overwrite_with_longer_string") {
    auto str = FixedString<10>("short");
    str = "muchlongerstring";
    REQUIRE(std::string(str.get_data()) == "muchlonger");
  }

  TEST_CASE("copy_construct_different_sizes") {
    auto str1 = FixedString<5>("12345");
    auto str2 = FixedString<10>(str1);
    auto str3 = FixedString<3>(str1);
    REQUIRE(std::string(str2.get_data()) == "12345");
    REQUIRE(std::string(str3.get_data()) == "123");
  }

  TEST_CASE("assignment_chain") {
    auto str1 = FixedString<10>("first");
    auto str2 = FixedString<10>();
    auto str3 = FixedString<10>();
    str3 = str2 = str1;
    REQUIRE(std::string(str1.get_data()) == "first");
    REQUIRE(std::string(str2.get_data()) == "first");
    REQUIRE(std::string(str3.get_data()) == "first");
  }

  TEST_CASE("compare_empty_strings") {
    auto str1 = FixedString<10>("");
    auto str2 = FixedString<20>("");
    REQUIRE(str1 == str2);
    REQUIRE(!(str1 < str2));
    REQUIRE(!(str1 > str2));
  }

  TEST_CASE("reset_and_reassign") {
    auto str = FixedString<10>("initial");
    str.reset();
    REQUIRE(str.is_empty());
    str = "reassigned";
    REQUIRE(std::string(str.get_data()) == "reassigned");
  }

  TEST_CASE("single_character_string") {
    auto str = FixedString<10>("a");
    REQUIRE(!str.is_empty());
    REQUIRE(std::string(str.get_data()) == "a");
  }

  TEST_CASE("exact_size_fit") {
    auto str = FixedString<5>("12345");
    REQUIRE(std::string(str.get_data()) == "12345");
  }

  TEST_CASE("spaceship_with_string_view") {
    auto str = FixedString<10>("test");
    REQUIRE((str <=> std::string_view("test")) ==
      std::strong_ordering::equal);
    REQUIRE((str <=> std::string_view("aaa")) ==
      std::strong_ordering::greater);
    REQUIRE((str <=> std::string_view("zzz")) ==
      std::strong_ordering::less);
  }

  TEST_CASE("multiple_resets") {
    auto str = FixedString<10>("test");
    str.reset();
    str.reset();
    str.reset();
    REQUIRE(str.is_empty());
  }

  TEST_CASE("assign_empty_string") {
    auto str = FixedString<10>("nonempty");
    str = "";
    REQUIRE(str.is_empty());
  }

  TEST_CASE("stream") {
    auto str = FixedString<10>("output");
    REQUIRE(to_string(str) == "output");
    test_round_trip_shuttle(str);
  }
}
