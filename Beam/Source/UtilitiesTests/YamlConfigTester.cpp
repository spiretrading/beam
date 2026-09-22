#include <map>
#include <doctest/doctest.h>
#include "Beam/Utilities/YamlConfig.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::gregorian;
using namespace boost::posix_time;

namespace {
  YAML::Node load_yaml(const std::map<std::string, std::string>& sources) {
    auto root = std::filesystem::absolute("yaml_config_tests");
    auto loader = Details::YamlConfigLoader([&] (const auto& path) {
      auto name = path.lexically_relative(root).generic_string();
      auto source = sources.find(name);
      if(source == sources.end()) {
        throw std::runtime_error("Missing YAML source: " + name);
      }
      return YAML::Load(source->second);
    });
    return loader.load(root / "config.yml", "");
  }
}

TEST_SUITE("YamlConfig") {
  TEST_CASE("include") {
    SUBCASE("root") {
      auto node = load_yaml({
        {"config.yml", "!include common.yml"},
        {"common.yml", "spin: {username: AUOS}"}});
      REQUIRE(node["spin"]["username"].as<std::string>() == "AUOS");
    }
    SUBCASE("nested") {
      auto node = load_yaml({
        {"config.yml", "spin: !include settings/spin.yml\n"
          "feeds: [!include settings/feeds.yml, !include settings/feeds.yml]"},
        {"settings/spin.yml", "username: !include ../username.yml"},
        {"settings/feeds.yml", "[one, two]"},
        {"username.yml", "AUOS"}});
      REQUIRE(node["spin"]["username"].as<std::string>() == "AUOS");
      REQUIRE(node["feeds"].size() == 2);
      REQUIRE(node["feeds"][0].IsSequence());
      REQUIRE(node["feeds"][0][1].as<std::string>() == "two");
      REQUIRE(node["feeds"][1][0].as<std::string>() == "one");
    }
    SUBCASE("null") {
      auto node = load_yaml({
        {"config.yml", "value: !include null.yml"}, {"null.yml", "null"}});
      REQUIRE(node["value"].IsNull());
    }
  }

  TEST_CASE("include_pointer") {
    SUBCASE("selected_subtree") {
      auto node = load_yaml({
        {"config.yml", "!include 'settings/common.yml#/spin'"},
        {"settings/common.yml", "spin: {username: !include username.yml}\n"
          "unused: !include missing.yml"},
        {"settings/username.yml", "AUOS"}});
      REQUIRE(node.size() == 1);
      REQUIRE(node["username"].as<std::string>() == "AUOS");
    }
    SUBCASE("tokens") {
      auto node = load_yaml({
        {"config.yml", "empty: !include 'common.yml#/'\n"
          "slash: !include 'common.yml#/a~1b'\n"
          "tilde: !include 'common.yml#/m~0n'\n"
          "escaped: !include 'common.yml#/a~01b'\n"
          "space: !include 'common.yml#/a%20b'\n"
          "index: !include 'common.yml#/feeds/1/address'\n"
          "whole: !include 'common.yml#'"},
        {"common.yml", "'': empty\n'a/b': slash\n'm~n': tilde\n"
          "'a~1b': escaped\n'a b': space\n"
          "feeds: [{address: first}, {address: second}]"}});
      for(auto name : {"empty", "slash", "tilde", "escaped", "space"}) {
        REQUIRE(node[name].as<std::string>() == name);
      }
      REQUIRE(node["index"].as<std::string>() == "second");
      REQUIRE(node["whole"]["feeds"].size() == 2);
    }
    SUBCASE("same_file") {
      auto node = load_yaml({{"config.yml",
        "defaults: {username: AUOS}\nspin: !include '#/defaults'"}});
      REQUIRE(node["spin"]["username"].as<std::string>() == "AUOS");
    }
  }

  TEST_CASE("include_invalid") {
    for(auto source : {"!include ''", "!include []", "!include {}",
        "!include missing.yml", "!include 'common.yml#spin'",
        "!include 'common.yml#/missing'", "!include 'common.yml#/bad~2key'",
        "!include 'common.yml#/bad~'", "!include 'common.yml#/%'",
        "!include 'common.yml#/%GG'", "!include 'common.yml#/feeds/01'",
        "!include 'common.yml#/feeds/-'", "!include 'common.yml#/feeds/2'",
        "!include 'common.yml#/feeds/-1'", "!include 'common.yml#/feeds/'",
        "!include 'common.yml#/feeds/999999999999999999999999999999'",
        "!include 'common.yml#/feeds/0/address/extra'"}) {
      CAPTURE(std::string_view(source));
      REQUIRE_THROWS_AS(load_yaml({{"config.yml", source},
        {"common.yml", "feeds: [{address: first}]"}}), std::runtime_error);
    }
    REQUIRE_THROWS_AS(load_yaml({
      {"config.yml", "!include 'common.yml#/key'"},
      {"common.yml", "key: one\nkey: two"}}), std::runtime_error);
  }

  TEST_CASE("include_cycle") {
    SUBCASE("files") {
      REQUIRE_THROWS_AS(load_yaml({
        {"config.yml", "!include nested/other.yml"},
        {"nested/other.yml", "!include ../config.yml"}}), std::runtime_error);
    }
    SUBCASE("pointers") {
      REQUIRE_THROWS_AS(load_yaml({{"config.yml",
        "one: !include '#/two'\ntwo: !include '#/one'"}}), std::runtime_error);
    }
  }

  TEST_CASE("include_error_context") {
    for(auto source : {"!include missing.yml", "[invalid"}) {
      CAPTURE(std::string_view(source));
      try {
        load_yaml({{"config.yml", "!include nested/other.yml"},
          {"nested/other.yml", source}});
        FAIL("Expected a YAML load error.");
      } catch(const std::runtime_error& e) {
        auto message = std::string(e.what());
        REQUIRE(message.find("config.yml") != std::string::npos);
        REQUIRE(message.find("other.yml") != std::string::npos);
      }
    }
  }

  TEST_CASE("include_alias") {
    auto node = load_yaml({
      {"config.yml", "spin: &spin !include spin.yml\ncopy: *spin\n"
        "recursive: &recursive {self: *recursive, value: !include value.yml}\n"
        "literal: '!include missing.yml'"},
      {"spin.yml", "{username: AUOS}"}, {"value.yml", "42"}});
    REQUIRE(node["spin"]["username"].as<std::string>() == "AUOS");
    REQUIRE(node["copy"].is(node["spin"]));
    REQUIRE(node["recursive"]["self"].is(node["recursive"]));
    REQUIRE(node["recursive"]["value"].as<int>() == 42);
    REQUIRE(node["literal"].as<std::string>() == "!include missing.yml");
  }

  TEST_CASE("merge") {
    for(auto source : {"<<: !include common.yml\nunit: 2",
        "unit: 2\n<<: !include common.yml"}) {
      CAPTURE(std::string_view(source));
      auto node = load_yaml({{"config.yml", source},
        {"common.yml", "unit: 1\nsampling: 100ms"}});
      REQUIRE(node["unit"].as<int>() == 2);
      REQUIRE(node["sampling"].as<std::string>() == "100ms");
      REQUIRE(!node["<<"]);
    }
  }

  TEST_CASE("merge_nested") {
    auto node = load_yaml({
      {"config.yml", "<<: !include common.yml\n"
        "spin:\n  <<: !include 'sessions.yml#/redundant'\n"
        "  session_sub_id: '0009'\n"
        "feeds: [{<<: !include 'sessions.yml#/feed', unit: 2}]"},
      {"common.yml", "<<: !include defaults.yml\n"
        "spin: {username: common, address: original}"},
      {"defaults.yml", "sampling: 100ms\nspin: {password: shared}"},
      {"sessions.yml", "redundant: {username: AUOS, address: redundant}\n"
        "feed: {unit: 1, venue: CXA}"}});
    REQUIRE(node["sampling"].as<std::string>() == "100ms");
    REQUIRE(node["spin"].size() == 4);
    REQUIRE(node["spin"]["username"].as<std::string>() == "AUOS");
    REQUIRE(node["spin"]["password"].as<std::string>() == "shared");
    REQUIRE(node["spin"]["address"].as<std::string>() == "redundant");
    REQUIRE(node["spin"]["session_sub_id"].as<std::string>() == "0009");
    REQUIRE(node["feeds"][0].size() == 2);
    REQUIRE(node["feeds"][0]["unit"].as<int>() == 2);
    REQUIRE(node["feeds"][0]["venue"].as<std::string>() == "CXA");
    REQUIRE(!node["<<"]);
  }

  TEST_CASE("merge_replacements") {
    auto node = load_yaml({
      {"config.yml", "<<: !include common.yml\n"
        "scalar: local\nsequence: [3]\nempty: []\nnull: null\n"
        "mapping: {local: 2}\nretained: {}"},
      {"common.yml", "scalar: {default: 1}\nsequence: [1, 2]\n"
        "empty: [1]\nnull: {default: 1}\nmapping: original\n"
        "retained: {default: 1}"}});
    REQUIRE(node["scalar"].as<std::string>() == "local");
    REQUIRE(node["sequence"].size() == 1);
    REQUIRE(node["sequence"][0].as<int>() == 3);
    REQUIRE(node["empty"].IsSequence());
    REQUIRE(node["empty"].size() == 0);
    REQUIRE(node["null"].IsNull());
    REQUIRE(node["mapping"]["local"].as<int>() == 2);
    REQUIRE(node["retained"]["default"].as<int>() == 1);
  }

  TEST_CASE("merge_alias") {
    auto node = load_yaml({{"config.yml",
      "defaults: &defaults\n"
      "  spin: &spin {username: AUOS, password: shared}\n"
      "  other: *spin\n"
      "first: {<<: *defaults, spin: {password: first}}\n"
      "second: {<<: *defaults, spin: {password: second}}\n"
      "literal: {'<<': retained}\n"
      "explicit: {!!merge '<<': {unit: 1}, unit: 2}"}});
    REQUIRE(node["defaults"]["spin"]["password"].as<std::string>() ==
      "shared");
    REQUIRE(node["first"]["spin"]["password"].as<std::string>() == "first");
    REQUIRE(node["second"]["spin"]["password"].as<std::string>() == "second");
    REQUIRE(node["first"]["spin"]["username"].as<std::string>() == "AUOS");
    REQUIRE(node["first"]["other"]["password"].as<std::string>() == "shared");
    REQUIRE(node["literal"]["<<"].as<std::string>() == "retained");
    REQUIRE(node["explicit"]["unit"].as<int>() == 2);
    REQUIRE(!node["first"]["<<"]);
    REQUIRE(!node["explicit"]["<<"]);
  }

  TEST_CASE("merge_invalid") {
    for(auto source : {"<<: null", "<<: text", "<<: [{unit: 1}]",
        "<<: {unit: 1}\n<<: {unit: 2}", "&self {<<: *self}",
        "&parent {child: {<<: *parent}}",
        "defaults: &defaults {self: *defaults}\n"
        "local: &local {<<: *defaults, self: *local}"}) {
      CAPTURE(std::string_view(source));
      REQUIRE_THROWS_AS(
        load_yaml({{"config.yml", source}}), std::runtime_error);
    }
    REQUIRE_THROWS_AS(load_yaml({
      {"config.yml", "<<: !include 'common.yml#/values'"},
      {"common.yml", "values: [1, 2]"}}), std::runtime_error);
  }

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

    SUBCASE("whitespace") {
      auto node = YAML::Node(" \t123 \n");
      REQUIRE(extract<int>(node) == 123);
    }
  }

  TEST_CASE("extract_invalid_int") {
    for(auto source : {"1.5", "1junk", "", "   ", "invalid",
        "999999999999999999999999"}) {
      CAPTURE(std::string_view(source));
      auto node = YAML::Node(source);
      REQUIRE_THROWS_AS(extract<int>(node), AssertionException);
    }
  }

  TEST_CASE("extract_range") {
    auto node = YAML::Load("value: 5");
    for(auto value : {1, 5, 10}) {
      node["value"] = value;
      REQUIRE(extract<int>(node, "value", 1, 10) == value);
    }
    for(auto value : {0, 11}) {
      node["value"] = value;
      REQUIRE_THROWS_AS(
        extract<int>(node, "value", 1, 10), AssertionException);
    }
    node["value"] = 5;
    REQUIRE(extract<int>(node, "value", 5, 5) == 5);
    REQUIRE_THROWS_AS(
      extract<int>(node, "value", 10, 1), AssertionException);
    node["value"] = "5junk";
    REQUIRE_THROWS_AS(
      extract<int>(node, "value", 1, 10), AssertionException);
    node.remove("value");
    REQUIRE_THROWS_AS(
      extract<int>(node, "value", 1, 10), AssertionException);
  }

  TEST_CASE("extract_default_range") {
    auto node = YAML::Load("value: 5");
    SUBCASE("supplied") {
      for(auto value : {1, 5, 10}) {
        node["value"] = value;
        REQUIRE(extract<int>(node, "value", 7, 1, 10) == value);
      }
      for(auto value : {0, 11}) {
        node["value"] = value;
        REQUIRE_THROWS_AS(
          extract<int>(node, "value", 7, 1, 10), AssertionException);
      }
      node["value"] = 5;
      REQUIRE(extract<int>(node, "value", 0, 1, 10) == 5);
      REQUIRE(extract<int>(node, "value", 5, 5, 5) == 5);
      REQUIRE_THROWS_AS(
        extract<int>(node, "value", 5, 10, 1), AssertionException);
    }
    SUBCASE("missing") {
      node.remove("value");
      for(auto value : {1, 5, 10}) {
        REQUIRE(extract<int>(node, "value", value, 1, 10) == value);
      }
      for(auto value : {0, 11}) {
        REQUIRE_THROWS_AS(
          extract<int>(node, "value", value, 1, 10), AssertionException);
      }
      REQUIRE(extract<int>(node, "value", 5, 5, 5) == 5);
      REQUIRE_THROWS_AS(
        extract<int>(node, "value", 5, 10, 1), AssertionException);
    }
    SUBCASE("malformed") {
      for(auto value : {"5junk", "5.5", ""}) {
        CAPTURE(std::string_view(value));
        node["value"] = value;
        REQUIRE_THROWS_AS(
          extract<int>(node, "value", 5, 1, 10), AssertionException);
      }
      node["value"] = YAML::Node(YAML::NodeType::Null);
      REQUIRE_THROWS_AS(
        extract<int>(node, "value", 5, 1, 10), AssertionException);
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

  TEST_CASE("extract_invalid_double") {
    for(auto source : {"3.14junk", "3.14.1", "1e", "1e9999", "invalid",
        "", "   "}) {
      CAPTURE(std::string_view(source));
      auto node = YAML::Node(source);
      REQUIRE_THROWS_AS(extract<double>(node), AssertionException);
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
