#include <doctest/doctest.h>
#include "Beam/Codecs/CodedWriter.hpp"
#include "Beam/CodecsTests/ReverseEncoder.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Codecs;
using namespace Beam::Codecs::Tests;
using namespace Beam::IO;

TEST_SUITE("CodedWriter") {
  TEST_CASE("single_byte") {
    auto pipedReader = PipedReader<SharedBuffer>();
    auto pipedWriter = PipedWriter<SharedBuffer>(Ref(pipedReader));
    auto codedWriter = CodedWriter<PipedWriter<SharedBuffer>*, ReverseEncoder>(
      &pipedWriter, ReverseEncoder());
    {
      codedWriter.Write(BufferFromString<SharedBuffer>("a"));
      auto readBuffer = SharedBuffer();
      REQUIRE(pipedReader.Read(Store(readBuffer)) == 1);
      REQUIRE(readBuffer.GetSize() == 1);
      REQUIRE(std::string(readBuffer.GetData(), readBuffer.GetSize()) == "a");
    }
    {
      codedWriter.Write(BufferFromString<SharedBuffer>("b"));
      auto readBuffer = SharedBuffer();
      REQUIRE(pipedReader.Read(Store(readBuffer)) == 1);
      REQUIRE(readBuffer.GetSize() == 1);
      REQUIRE(std::string(readBuffer.GetData(), readBuffer.GetSize()) == "b");
    }
  }

  TEST_CASE("write") {
    auto pipedReader = PipedReader<SharedBuffer>();
    auto pipedWriter = PipedWriter<SharedBuffer>(Ref(pipedReader));
    auto codedWriter = CodedWriter<PipedWriter<SharedBuffer>*, ReverseEncoder>(
      &pipedWriter, ReverseEncoder());
    {
      auto message = std::string("hello");
      auto reversedMessage = std::string(message.rbegin(), message.rend());
      codedWriter.Write(BufferFromString<SharedBuffer>(message));
      auto readBuffer = SharedBuffer();
      REQUIRE(pipedReader.Read(Store(readBuffer)) == reversedMessage.size());
      REQUIRE(readBuffer.GetSize() == reversedMessage.size());
      REQUIRE(std::string(readBuffer.GetData(), readBuffer.GetSize()) ==
        reversedMessage);
    }
    {
      auto message = std::string("world");
      auto reversedMessage = std::string(message.rbegin(), message.rend());
      codedWriter.Write(BufferFromString<SharedBuffer>(message));
      auto readBuffer = SharedBuffer();
      REQUIRE(pipedReader.Read(Store(readBuffer)) == reversedMessage.size());
      REQUIRE(readBuffer.GetSize() == reversedMessage.size());
      REQUIRE(std::string(readBuffer.GetData(), readBuffer.GetSize()) ==
        reversedMessage);
    }
  }
}
