#include <doctest/doctest.h>
#include "Beam/Codecs/CodedReader.hpp"
#include "Beam/Codecs/ZLibDecoder.hpp"
#include "Beam/Codecs/ZLibEncoder.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/IOTests/TestReader.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("CodedReader") {
  TEST_CASE("empty") {
    auto reader =
      CodedReader(BufferReader(from<SharedBuffer>("")), ReverseDecoder());
    auto buffer = SharedBuffer();
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
  }

  TEST_CASE("empty_decoded_block") {
    auto source = PipedReader();
    auto writer = PipedWriter(Ref(source));
    auto encoder = ZLibEncoder();
    auto encoded = SharedBuffer();
    encoder.encode(SharedBuffer(), out(encoded));
    writer.write(encoded);
    auto reader = CodedReader(&source, ZLibDecoder());
    auto buffer = SharedBuffer();
    SUBCASE("end_of_file") {
      writer.close();
      REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
    }
    SUBCASE("following_block") {
      reset(encoded);
      encoder.encode(from<SharedBuffer>("hello"), out(encoded));
      writer.write(encoded);
      writer.close();
      REQUIRE(reader.read(out(buffer)) == 5);
      REQUIRE(buffer == "hello");
      REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
    }
  }

  TEST_CASE("read_failure") {
    auto operations = std::make_shared<TestReader::Queue>();
    auto source = TestReader(operations);
    auto reader = CodedReader(&source, ZLibDecoder());
    auto buffer = SharedBuffer();
    auto routine = RoutineHandler(spawn([&] {
      REQUIRE_THROWS_AS(reader.read(out(buffer)), IOException);
      REQUIRE_THROWS_AS(reader.read(out(buffer)), IOException);
      REQUIRE_FALSE(reader.poll());
    }));
    auto operation = operations->pop();
    auto& read = std::get<TestReader::ReadOperation>(*operation);
    SUBCASE("decoder") {
      read.m_result.set(from<SharedBuffer>("invalid"));
    }
    SUBCASE("source") {
      read.m_result.set(std::make_exception_ptr(IOException()));
    }
    flush_pending_routines();
    auto unexpected_operation = operations->try_pop();
    source.close();
    routine.wait();
    REQUIRE(read.m_size == std::numeric_limits<std::size_t>::max());
    REQUIRE(!unexpected_operation);
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
