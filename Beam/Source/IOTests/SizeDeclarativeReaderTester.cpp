#include <boost/endian.hpp>
#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SizeDeclarativeReader.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace boost;
using namespace boost::endian;
using namespace Beam;

TEST_SUITE("SizeDeclarativeReader") {
  TEST_CASE("empty_source") {
    auto reader = SizeDeclarativeReader(BufferReader(from<SharedBuffer>("")));
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }

  TEST_CASE("single_read") {
    auto piped_reader = PipedReader();
    auto piped_writer = PipedWriter(Ref(piped_reader));
    auto reader = SizeDeclarativeReader(&piped_reader);
    auto message = from<SharedBuffer>("hello world");
    write(piped_writer,
      native_to_little(static_cast<std::uint32_t>(message.get_size())));
    piped_writer.write(message);
    auto retrieved_message = SharedBuffer();
    REQUIRE(reader.read(out(retrieved_message)) == message.get_size());
    REQUIRE(retrieved_message == message);
  }

  TEST_CASE("multi_read_message") {
    auto piped_reader = PipedReader();
    auto piped_writer = PipedWriter(Ref(piped_reader));
    auto reader = SizeDeclarativeReader(&piped_reader);
    auto first_fragment = from<SharedBuffer>("hello");
    auto second_fragment = from<SharedBuffer>(" world");
    auto message = first_fragment;
    append(message, second_fragment);
    write(piped_writer,
      native_to_little(static_cast<std::uint32_t>(message.get_size())));
    piped_writer.write(first_fragment);
    auto retrieved_message = SharedBuffer();
    auto read_result = size_t();
    auto task = RoutineHandler(spawn([&] {
      read_result = reader.read(out(retrieved_message));
    }));
    piped_writer.write(second_fragment);
    task.wait();
    REQUIRE(read_result == message.get_size());
    REQUIRE(retrieved_message == message);
  }
}
