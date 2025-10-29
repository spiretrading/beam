#include <string>
#include <doctest/doctest.h>
#include "Beam/Queues/TaggedQueueReader.hpp"

using namespace Beam;

TEST_SUITE("TaggedQueueReader") {
  TEST_CASE("single_push_pop") {
    auto reader = TaggedQueueReader<int, std::string>();
    auto slot = reader.get_slot(1);
    slot.push("hello");
    auto value = reader.pop();
    REQUIRE(value.m_key == 1);
    REQUIRE(value.m_value == "hello");
  }

  TEST_CASE("try_pop_empty") {
    auto reader = TaggedQueueReader<int, std::string>();
    auto opt = reader.try_pop();
    REQUIRE(!opt);
  }

  TEST_CASE("multiple_slots_order") {
    auto reader = TaggedQueueReader<int, std::string>();
    auto slot1 = reader.get_slot(1);
    auto slot2 = reader.get_slot(2);
    slot1.push("a");
    slot2.push("b");
    slot1.push("c");
    auto first = reader.pop();
    auto second = reader.pop();
    auto third = reader.pop();
    REQUIRE(first.m_key == 1);
    REQUIRE(first.m_value == "a");
    REQUIRE(second.m_key == 2);
    REQUIRE(second.m_value == "b");
    REQUIRE(third.m_key == 1);
    REQUIRE(third.m_value == "c");
  }

  TEST_CASE("move_value_push") {
    auto reader = TaggedQueueReader<int, std::string>();
    auto slot = reader.get_slot(5);
    slot.push("moved");
    auto value = reader.pop();
    REQUIRE(value.m_key == 5);
    REQUIRE(value.m_value == "moved");
  }

  TEST_CASE("close_unblocks_try_pop") {
    auto reader = TaggedQueueReader<int, std::string>();
    reader.close(std::make_exception_ptr(std::runtime_error("broken")));
    auto opt = reader.try_pop();
    REQUIRE(!opt);
  }
}
