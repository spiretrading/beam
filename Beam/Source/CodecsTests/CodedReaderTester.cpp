#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Codecs/CodedReader.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("CodedReader") {
  TEST_CASE("empty") {
    auto reader =
      CodedReader(BufferReader(from<SharedBuffer>("")), ReverseDecoder());
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }

  TEST_CASE("single_byte") {
    auto reader =
      CodedReader(BufferReader(from<SharedBuffer>("a")), ReverseDecoder());
    auto buffer = SharedBuffer();
    REQUIRE(reader.read(out(buffer)) == 1);
    REQUIRE(buffer == "a");
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }

  TEST_CASE("read") {
    auto message = std::string("hello world");
    auto reverse = std::string("dlrow olleh");
    auto reader =
      CodedReader(BufferReader(from<SharedBuffer>(message)), ReverseDecoder());
    auto buffer = SharedBuffer();
    REQUIRE(reader.read(out(buffer)) == static_cast<int>(reverse.size()));
    REQUIRE(buffer == reverse);
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }

  TEST_CASE("read_some") {
    auto message = std::string("helloworld");
    auto first_reverse = std::string("dlrow");
    auto second_reverse = std::string("olleh");
    auto reader =
      CodedReader(BufferReader(from<SharedBuffer>(message)), ReverseDecoder());
    auto buffer = SharedBuffer();
    REQUIRE(reader.read(out(buffer), static_cast<int>(first_reverse.size())) ==
      static_cast<int>(first_reverse.size()));
    REQUIRE(buffer == first_reverse);
    reset(buffer);
    REQUIRE(reader.read(out(buffer),
      static_cast<int>(second_reverse.size())) ==
        static_cast<int>(second_reverse.size()));
    REQUIRE(buffer == second_reverse);
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }
}
