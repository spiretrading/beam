#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IO/SizeDeclarativeReader.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;

TEST_SUITE("SizeDeclarativeReader") {
  TEST_CASE("TestEmptySource") {
    auto reader = SizeDeclarativeReader<BufferReader<SharedBuffer>>(
      Initialize(BufferFromString<SharedBuffer>("")));
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(reader.Read(Store(buffer)), EndOfFileException);
  }

  TEST_CASE("TestSingleReadMessage") {
    auto pipedReader = PipedReader<SharedBuffer>();
    auto pipedWriter = PipedWriter<SharedBuffer>(Ref(pipedReader));
    auto reader = SizeDeclarativeReader<PipedReader<SharedBuffer>*>(
      &pipedReader);

    // Build the message.
    auto message = std::string("hello world");
    auto size = ToLittleEndian(static_cast<std::uint32_t>(message.size()));
    pipedWriter.Write(&size, 4);
    pipedWriter.Write(message.c_str(), message.size());

    // Extract the message and test that it is equal to the original message.
    auto retrievedMessage = SharedBuffer();
    REQUIRE(reader.Read(Store(retrievedMessage)) == message.size());
    REQUIRE(retrievedMessage.GetSize() == message.size());
    REQUIRE(std::string(retrievedMessage.GetData(), message.size()) == message);
  }

  TEST_CASE("TestMultiReadMessage") {
    auto pipedReader = PipedReader<SharedBuffer>();
    auto pipedWriter = PipedWriter<SharedBuffer>(Ref(pipedReader));
    auto reader = SizeDeclarativeReader<PipedReader<SharedBuffer>*>(
      &pipedReader);

    // Build the message and write the first message fragment.
    auto firstFragment = std::string("hello");
    auto secondFragment = std::string(" world");
    auto message = firstFragment + secondFragment;
    auto size = ToLittleEndian(static_cast<std::uint32_t>(message.size()));
    pipedWriter.Write(&size, 4);
    pipedWriter.Write(firstFragment.c_str(), firstFragment.size());

    // Begin reading the message.
    auto retrievedMessage = SharedBuffer();
    auto readResult = size_t();
    auto task = RoutineHandler(Spawn(
      [&] {
        readResult = reader.Read(Store(retrievedMessage));
      }));

    // Write the second message fragment.
    pipedWriter.Write(secondFragment.c_str(), secondFragment.size());
    task.Wait();

    // Extract the message and test that it is equal to the original message.
    REQUIRE(readResult == message.size());
    REQUIRE(retrievedMessage.GetSize() == message.size());
    REQUIRE(std::string(retrievedMessage.GetData(), message.size()) == message);
  }
}
