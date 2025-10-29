#include <doctest/doctest.h>
#include "Beam/Codecs/CodedWriter.hpp"
#include "Beam/CodecsTests/ReverseEncoder.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("CodedWriter") {
  TEST_CASE("single_byte") {
    auto piped_reader = PipedReader();
    auto piped_writer = PipedWriter(Ref(piped_reader));
    auto coded_writer = CodedWriter(&piped_writer, ReverseEncoder());
    {
      coded_writer.write(from<SharedBuffer>("a"));
      auto read_buffer = SharedBuffer();
      REQUIRE(piped_reader.read(out(read_buffer)) == 1);
      REQUIRE(read_buffer.get_size() == 1);
      REQUIRE(read_buffer == "a");
    }
    {
      coded_writer.write(from<SharedBuffer>("b"));
      auto read_buffer = SharedBuffer();
      REQUIRE(piped_reader.read(out(read_buffer)) == 1);
      REQUIRE(read_buffer.get_size() == 1);
      REQUIRE(read_buffer == "b");
    }
  }

  TEST_CASE("write") {
    auto piped_reader = PipedReader();
    auto piped_writer = PipedWriter(Ref(piped_reader));
    auto coded_writer = CodedWriter(&piped_writer, ReverseEncoder());
    {
      auto message = std::string("hello");
      auto reversed_message = std::string(message.rbegin(), message.rend());
      coded_writer.write(from<SharedBuffer>(message));
      auto read_buffer = SharedBuffer();
      REQUIRE(piped_reader.read(out(read_buffer)) == reversed_message.size());
      REQUIRE(read_buffer.get_size() == reversed_message.size());
      REQUIRE(read_buffer == reversed_message);
    }
    {
      auto message = std::string("world");
      auto reversed_message = std::string(message.rbegin(), message.rend());
      coded_writer.write(from<SharedBuffer>(message));
      auto read_buffer = SharedBuffer();
      REQUIRE(piped_reader.read(out(read_buffer)) == reversed_message.size());
      REQUIRE(read_buffer.get_size() == reversed_message.size());
      REQUIRE(read_buffer == reversed_message);
    }
  }
}
