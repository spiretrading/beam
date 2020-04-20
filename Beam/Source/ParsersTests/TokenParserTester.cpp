#include <doctest/doctest.h>
#include "Beam/Parsers/Operators.hpp"
#include "Beam/Parsers/TokenParser.hpp"
#include "Beam/Parsers/Types.hpp"
#include "Beam/Parsers/ReaderParserStream.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Parsers;

TEST_SUITE("TokenParser") {
  TEST_CASE("no_space_token") {
    auto parser = Token(+alpha_p);
    auto source = ParserStreamFromString("hello");
    auto value = std::string();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == "hello");
  }

  TEST_CASE("one_token") {
    auto parser = Token(+alpha_p);
    auto source = ParserStreamFromString("   hello");
    auto value = std::string();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value == "hello");
  }

  TEST_CASE("two_tokens") {
    auto parser = Token(+alpha_p) >> Token(*alpha_p);
    auto source = ParserStreamFromString("   hello   world");
    auto value = std::tuple<std::string, std::string>();
    REQUIRE(parser.Read(source, value));
    REQUIRE(std::get<0>(value) == "hello");
    REQUIRE(std::get<1>(value) == "world");
  }

  TEST_CASE("star_tokens") {
    auto parser = *Token(+alpha_p);
    auto source = ParserStreamFromString("   hello   world goodbye   sky");
    auto value = std::vector<std::string>();
    REQUIRE(parser.Read(source, value));
    REQUIRE(value.size() == 4);
    REQUIRE(value[0] == "hello");
    REQUIRE(value[1] == "world");
    REQUIRE(value[2] == "goodbye");
    REQUIRE(value[3] == "sky");
  }

  TEST_CASE("chaining_tokens") {
    auto parser = tokenize >> +alpha_p >> +alpha_p >> +alpha_p;
    auto source = ParserStreamFromString("   hello   world goodbye");
    auto value = std::tuple<std::string, std::string, std::string>();
    REQUIRE(parser.Read(source, value));
    REQUIRE(std::get<0>(value) == "hello");
    REQUIRE(std::get<1>(value) == "world");
    REQUIRE(std::get<2>(value) == "goodbye");
  }
}
