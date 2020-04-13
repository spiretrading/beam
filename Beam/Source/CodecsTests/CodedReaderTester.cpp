#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Codecs/CodedReader.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;

TEST_SUITE("CodedReader") {
  TEST_CASE("empty") {
    auto codedReader = CodedReader<SharedBuffer, BufferReader<SharedBuffer>,
      ReverseDecoder>(Initialize(BufferFromString<SharedBuffer>("")),
      ReverseDecoder());
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(codedReader.Read(Store(buffer)), EndOfFileException);
  }

  TEST_CASE("single_byte") {
    auto codedReader = CodedReader<SharedBuffer, BufferReader<SharedBuffer>,
      ReverseDecoder>(Initialize(BufferFromString<SharedBuffer>("a")),
      ReverseDecoder());
    auto buffer = SharedBuffer();
    REQUIRE(codedReader.Read(Store(buffer)) == 1);
    REQUIRE(buffer.GetSize() == 1);
    REQUIRE(buffer.GetData()[0] == 'a');
    REQUIRE_THROWS_AS(codedReader.Read(Store(buffer)), EndOfFileException);
  }

  TEST_CASE("read") {
    auto message = std::string("hello world");
    auto reverse = std::string("dlrow olleh");
    auto codedReader = CodedReader<SharedBuffer, BufferReader<SharedBuffer>,
      ReverseDecoder>(Initialize(BufferFromString<SharedBuffer>(message)),
      ReverseDecoder());
    auto buffer = SharedBuffer();
    REQUIRE(codedReader.Read(Store(buffer)) ==
      static_cast<int>(reverse.size()));
    REQUIRE(buffer.GetSize() == static_cast<int>(reverse.size()));
    REQUIRE(std::string(buffer.GetData(), buffer.GetSize()) == reverse);
    REQUIRE_THROWS_AS(codedReader.Read(Store(buffer)), EndOfFileException);
  }

  TEST_CASE("read_some") {
    auto message = std::string("helloworld");
    auto firstReverse = std::string("dlrow");
    auto secondReverse = std::string("olleh");
    auto codedReader = CodedReader<SharedBuffer, BufferReader<SharedBuffer>,
      ReverseDecoder>(Initialize(BufferFromString<SharedBuffer>(message)),
      ReverseDecoder());
    auto buffer = SharedBuffer();
    REQUIRE(codedReader.Read(Store(buffer),
      static_cast<int>(firstReverse.size())) ==
      static_cast<int>(firstReverse.size()));
    REQUIRE(buffer.GetSize() == static_cast<int>(firstReverse.size()));
    REQUIRE(std::string(buffer.GetData(), buffer.GetSize()) == firstReverse);
    buffer.Reset();
    REQUIRE(codedReader.Read(Store(buffer),
      static_cast<int>(secondReverse.size())) ==
      static_cast<int>(secondReverse.size()));
    REQUIRE(buffer.GetSize() == static_cast<int>(secondReverse.size()));
    REQUIRE(std::string(buffer.GetData(), buffer.GetSize()) == secondReverse);
    REQUIRE_THROWS_AS(codedReader.Read(Store(buffer)), EndOfFileException);
  }
}
