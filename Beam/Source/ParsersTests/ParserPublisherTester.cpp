#include <doctest/doctest.h>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Parsers/DefaultParser.hpp"
#include "Beam/Parsers/ParserPublisher.hpp"
#include "Beam/Queues/Queue.hpp"

using namespace Beam;

TEST_SUITE("ParserPublisher") {
  TEST_CASE("publish_single_value_from_buffer_reader") {
    auto buffer = from<SharedBuffer>("123");
    auto publisher = ParserPublisher(BufferReader(from<SharedBuffer>("123")),
      default_parser<int>, ParserErrorPolicy::REPORT);
    auto queue = std::make_shared<Queue<int>>();
    publisher.monitor(queue);
    auto value = queue->pop();
    REQUIRE(value == 123);
  }

  TEST_CASE("publish_multiple_values_from_buffer_reader") {
    auto reader = std::make_shared<PipedReader>();
    auto writer = PipedWriter(Ref(*reader));
    auto publisher =
      ParserPublisher(reader, default_parser<int>, ParserErrorPolicy::REPORT);
    auto queue = std::make_shared<Queue<int>>();
    publisher.monitor(queue);
    writer.write(from<SharedBuffer>("1"));
    auto value = queue->pop();
    REQUIRE(value == 1);
    writer.write(from<SharedBuffer>("23"));
    value = queue->pop();
    REQUIRE(value == 23);
    writer.write(from<SharedBuffer>("456"));
    value = queue->pop();
    REQUIRE(value == 456);
    writer.close();
  }

  TEST_CASE("publish_anyparser_characters") {
    auto publisher = ParserPublisher(BufferReader(from<SharedBuffer>("abc")),
      AnyParser(), ParserErrorPolicy::REPORT);
    auto queue = std::make_shared<Queue<char>>();
    publisher.monitor(queue);
    auto value = queue->pop();
    REQUIRE(value == 'a');
    value = queue->pop();
    REQUIRE(value == 'b');
    value = queue->pop();
    REQUIRE(value == 'c');
  }

  TEST_CASE("empty_reader_produces_no_values") {
    auto publisher = ParserPublisher(BufferReader(from<SharedBuffer>("")),
      default_parser<int>, ParserErrorPolicy::REPORT);
    auto queue = std::make_shared<Queue<int>>();
    publisher.monitor(queue);
    REQUIRE_THROWS_AS(queue->pop(), PipeBrokenException);
  }
}
