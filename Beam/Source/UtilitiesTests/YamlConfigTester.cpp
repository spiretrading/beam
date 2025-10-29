#include <doctest/doctest.h>
#include "Beam/Utilities/YamlConfig.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

TEST_SUITE("YamlConfig") {
  TEST_CASE("extract_bool") {
    SUBCASE("true") {
      auto node = YAML::Load("true");
      auto value = extract<bool>(node);
      REQUIRE(value);
    }

    SUBCASE("false") {
      auto node = YAML::Load("false");
      auto value = extract<bool>(node);
      REQUIRE(!value);
    }
  }

  TEST_CASE("extract_int") {
    SUBCASE("positive") {
      auto node = YAML::Load("123");
      auto value = extract<int>(node);
      REQUIRE(value == 123);
    }

    SUBCASE("negative") {
      auto node = YAML::Load("-123");
      auto value = extract<int>(node);
      REQUIRE(value == -123);
    }
  }

  TEST_CASE("extract_double") {
    SUBCASE("positive") {
      auto node = YAML::Load("3.14159");
      auto value = extract<double>(node);
      REQUIRE(value == doctest::Approx(3.14159));
    }

    SUBCASE("negative") {
      auto node = YAML::Load("-2.71828");
      auto value = extract<double>(node);
      REQUIRE(value == doctest::Approx(-2.71828));
    }

    SUBCASE("scientific_notation") {
      auto node = YAML::Load("1.23e-4");
      auto value = extract<double>(node);
      REQUIRE(value == doctest::Approx(1.23e-4));
    }
  }

  TEST_CASE("extract_string") {
    SUBCASE("simple") {
      auto node = YAML::Load("hello");
      auto value = extract<std::string>(node);
      REQUIRE(value == "hello");
    }

    SUBCASE("with_whitespace") {
      auto node = YAML::Load("  hello world  ");
      auto value = extract<std::string>(node);
      REQUIRE(value == "hello world");
    }

    SUBCASE("empty") {
      auto node = YAML::Load("\"\"");
      auto value = extract<std::string>(node);
      REQUIRE(value == "");
    }

    SUBCASE("quoted") {
      auto node = YAML::Load("\"quoted string\"");
      auto value = extract<std::string>(node);
      REQUIRE(value == "quoted string");
    }
  }

  TEST_CASE("extract_rational") {
    SUBCASE("simple_fraction") {
      auto node = YAML::Load("0.75");
      auto value = extract<rational<int>>(node);
      REQUIRE(value.numerator() == 3);
      REQUIRE(value.denominator() == 4);
    }

    SUBCASE("whole_number") {
      auto node = YAML::Load("5");
      auto value = extract<rational<int>>(node);
      REQUIRE(value.numerator() == 5);
      REQUIRE(value.denominator() == 1);
    }

    SUBCASE("negative_fraction") {
      auto node = YAML::Load("-0.5");
      auto value = extract<rational<int>>(node);
      REQUIRE(value.numerator() == -1);
      REQUIRE(value.denominator() == 2);
    }
  }

  TEST_CASE("extract_vector") {
    SUBCASE("int_vector") {
      auto node = YAML::Load("[1, 2, 3, 4, 5]");
      auto value = extract<std::vector<int>>(node);
      REQUIRE(value.size() == 5);
      REQUIRE(value[0] == 1);
      REQUIRE(value[1] == 2);
      REQUIRE(value[2] == 3);
      REQUIRE(value[3] == 4);
      REQUIRE(value[4] == 5);
    }

    SUBCASE("empty_vector") {
      auto node = YAML::Load("[]");
      auto value = extract<std::vector<int>>(node);
      REQUIRE(value.empty());
    }

    SUBCASE("single_element") {
      auto node = YAML::Load("[42]");
      auto value = extract<std::vector<int>>(node);
      REQUIRE(value.size() == 1);
      REQUIRE(value[0] == 42);
    }
  }

  TEST_CASE("extract_time_duration") {
    SUBCASE("hours") {
      auto node = YAML::Load("2h");
      auto value = extract<time_duration>(node);
      REQUIRE(value == hours(2));
    }

    SUBCASE("minutes") {
      auto node = YAML::Load("30m");
      auto value = extract<time_duration>(node);
      REQUIRE(value == minutes(30));
    }

    SUBCASE("seconds") {
      auto node = YAML::Load("45s");
      auto value = extract<time_duration>(node);
      REQUIRE(value == seconds(45));
    }

    SUBCASE("milliseconds") {
      auto node = YAML::Load("500ms");
      auto value = extract<time_duration>(node);
      REQUIRE(value == milliseconds(500));
    }

    SUBCASE("microseconds") {
      auto node = YAML::Load("1000us");
      auto value = extract<time_duration>(node);
      REQUIRE(value == microseconds(1000));
    }

    SUBCASE("positive_infinity") {
      auto node = YAML::Load("infinity");
      auto value = extract<time_duration>(node);
      REQUIRE(value == pos_infin);
    }

    SUBCASE("negative_infinity") {
      auto node = YAML::Load("-infinity");
      auto value = extract<time_duration>(node);
      REQUIRE(value == neg_infin);
    }
  }

  TEST_CASE("extract_date") {
    SUBCASE("standard_format") {
      auto node = YAML::Load("2023-12-25");
      auto value = extract<date>(node);
      REQUIRE(value.year() == 2023);
      REQUIRE(value.month() == 12);
      REQUIRE(value.day() == 25);
    }

    SUBCASE("different_date") {
      auto node = YAML::Load("2024-01-01");
      auto value = extract<date>(node);
      REQUIRE(value.year() == 2024);
      REQUIRE(value.month() == 1);
      REQUIRE(value.day() == 1);
    }
  }

  TEST_CASE("extract_ptime") {
    SUBCASE("date_and_time") {
      auto node = YAML::Load("2023-12-25 14:30:00");
      auto value = extract<ptime>(node);
      REQUIRE(value.date().year() == 2023);
      REQUIRE(value.date().month() == 12);
      REQUIRE(value.date().day() == 25);
      REQUIRE(value.time_of_day().hours() == 14);
      REQUIRE(value.time_of_day().minutes() == 30);
      REQUIRE(value.time_of_day().seconds() == 0);
    }

    SUBCASE("midnight") {
      auto node = YAML::Load("2024-01-01 00:00:00");
      auto value = extract<ptime>(node);
      REQUIRE(value.date().year() == 2024);
      REQUIRE(value.date().month() == 1);
      REQUIRE(value.date().day() == 1);
      REQUIRE(value.time_of_day().hours() == 0);
      REQUIRE(value.time_of_day().minutes() == 0);
      REQUIRE(value.time_of_day().seconds() == 0);
    }
  }

  TEST_CASE("extract_ip_address") {
    SUBCASE("with_port") {
      auto node = YAML::Load("192.168.1.1:8080");
      auto value = extract<IpAddress>(node);
      REQUIRE(value.get_host() == "192.168.1.1");
      REQUIRE(value.get_port() == 8080);
    }

    SUBCASE("without_port") {
      auto node = YAML::Load("localhost");
      auto value = extract<IpAddress>(node);
      REQUIRE(value.get_host() == "localhost");
      REQUIRE(value.get_port() == 0);
    }

    SUBCASE("hostname_with_port") {
      auto node = YAML::Load("example.com:443");
      auto value = extract<IpAddress>(node);
      REQUIRE(value.get_host() == "example.com");
      REQUIRE(value.get_port() == 443);
    }
  }
}
