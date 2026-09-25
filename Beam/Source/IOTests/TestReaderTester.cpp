#include <future>
#include <doctest/doctest.h>
#include "Beam/IOTests/TestReader.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("TestReader") {
  TEST_CASE("read") {
    auto operations = std::make_shared<TestReader::Queue>();
    auto reader = TestReader(operations);
    auto buffer = from<SharedBuffer>("prefix");
    auto size = std::numeric_limits<std::size_t>::max();
    SUBCASE("unlimited") {}
    SUBCASE("limited") {
      size = 3;
    }
    auto result = std::async(std::launch::async, [&] {
      if(size == std::numeric_limits<std::size_t>::max()) {
        return reader.read(out(buffer));
      }
      return reader.read(out(buffer), size);
    });
    auto operation = operations->pop();
    auto& read = std::get<TestReader::ReadOperation>(*operation);
    read.m_result.set(from<SharedBuffer>("abc"));
    REQUIRE(result.get() == 3);
    REQUIRE(read.m_size == size);
    REQUIRE(buffer == "prefixabc");
  }

  TEST_CASE("poll") {
    auto operations = std::make_shared<TestReader::Queue>();
    auto reader = TestReader(operations);
    auto is_available = false;
    SUBCASE("unavailable") {}
    SUBCASE("available") {
      is_available = true;
    }
    auto result = std::async(std::launch::async, [&] {
      return std::as_const(reader).poll();
    });
    auto operation = operations->pop();
    std::get<TestReader::PollOperation>(*operation).m_result.set(is_available);
    REQUIRE(result.get() == is_available);
  }

  TEST_CASE("read_failure") {
    auto operations = std::make_shared<TestReader::Queue>();
    auto reader = TestReader(operations);
    auto buffer = SharedBuffer();
    auto result = std::async(std::launch::async, [&] {
      return reader.read(out(buffer));
    });
    auto operation = operations->pop();
    std::get<TestReader::ReadOperation>(*operation).m_result.set(
      std::make_exception_ptr(IOException()));
    REQUIRE_THROWS_AS(result.get(), IOException);
    REQUIRE(is_empty(buffer));
  }

  TEST_CASE("close") {
    auto operations = std::make_shared<TestReader::Queue>();
    auto reader = TestReader(operations);
    auto buffer = SharedBuffer();
    auto result = std::async(std::launch::async, [&] {
      return reader.read(out(buffer));
    });
    operations->pop();
    reader.close();
    REQUIRE_THROWS_AS(result.get(), EndOfFileException);
    REQUIRE_THROWS_AS(reader.read(out(buffer)), EndOfFileException);
    REQUIRE(!operations->try_pop());
  }
}
