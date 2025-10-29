#include <iostream>
#include <sstream>
#include <string>
#include <doctest/doctest.h>
#include "Beam/Utilities/ScopedStreamManipulator.hpp"

using namespace Beam;

namespace {
  struct TestContext {
    int m_value;
  };

  struct AnotherContext {
    std::string m_name;
  };

  template<typename T>
  auto get_context(std::ostream& stream) {
    auto pointer = stream.pword(ScopedStreamManipulator<T>::ID);
    return static_cast<T*>(pointer);
  }
}

TEST_SUITE("ScopedStreamManipulator") {
  TEST_CASE("installs_context_into_stream") {
    auto stream = std::stringstream();
    auto context = TestContext(42);
    {
      auto manipulator = ScopedStreamManipulator(stream, context);
      auto retrieved = get_context<TestContext>(stream);
      REQUIRE(retrieved);
      REQUIRE(retrieved->m_value == 42);
    }
  }

  TEST_CASE("removes_context_on_destruction") {
    auto stream = std::stringstream();
    auto context = TestContext(123);
    {
      auto manipulator = ScopedStreamManipulator(stream, context);
      auto retrieved = get_context<TestContext>(stream);
      REQUIRE(retrieved);
    }
    auto retrieved = get_context<TestContext>(stream);
    REQUIRE(!retrieved);
  }

  TEST_CASE("conversion_operator_returns_stream") {
    auto stream = std::stringstream();
    auto context = TestContext(99);
    auto manipulator = ScopedStreamManipulator(stream, context);
    auto& returned_stream = static_cast<std::ostream&>(manipulator);
    REQUIRE(&returned_stream == &stream);
  }

  TEST_CASE("context_survives_for_manipulator_lifetime") {
    auto stream = std::stringstream();
    auto context = TestContext(55);
    auto manipulator = ScopedStreamManipulator(stream, context);
    context.m_value = 77;
    auto retrieved = get_context<TestContext>(stream);
    REQUIRE(retrieved);
    REQUIRE(retrieved->m_value == 77);
  }

  TEST_CASE("multiple_contexts_with_different_types") {
    auto stream = std::stringstream();
    auto context1 = TestContext(10);
    auto context2 = AnotherContext("hello");
    {
      auto manipulator1 = ScopedStreamManipulator(stream, context1);
      auto manipulator2 = ScopedStreamManipulator(stream, context2);
      auto retrieved1 = get_context<TestContext>(stream);
      auto retrieved2 = get_context<AnotherContext>(stream);
      REQUIRE(retrieved1);
      REQUIRE(retrieved1->m_value == 10);
      REQUIRE(retrieved2);
      REQUIRE(retrieved2->m_name == "hello");
    }
    auto retrieved1 = get_context<TestContext>(stream);
    auto retrieved2 = get_context<AnotherContext>(stream);
    REQUIRE(!retrieved1);
    REQUIRE(!retrieved2);
  }

  TEST_CASE("nested_scopes_with_same_type") {
    auto stream = std::stringstream();
    auto context1 = TestContext(100);
    auto context2 = TestContext(200);
    {
      auto manipulator1 = ScopedStreamManipulator(stream, context1);
      auto retrieved = get_context<TestContext>(stream);
      REQUIRE(retrieved->m_value == 100);
      {
        auto manipulator2 = ScopedStreamManipulator(stream, context2);
        retrieved = get_context<TestContext>(stream);
        REQUIRE(retrieved->m_value == 200);
      }
      retrieved = get_context<TestContext>(stream);
      REQUIRE(retrieved->m_value == 100);
    }
    auto retrieved = get_context<TestContext>(stream);
    REQUIRE(!retrieved);
  }

  TEST_CASE("each_type_has_unique_id") {
    auto id1 = ScopedStreamManipulator<TestContext>::ID;
    auto id2 = ScopedStreamManipulator<AnotherContext>::ID;
    auto id3 = ScopedStreamManipulator<int>::ID;
    REQUIRE(id1 != id2);
    REQUIRE(id1 != id3);
    REQUIRE(id2 != id3);
  }

  TEST_CASE("id_is_consistent_across_calls") {
    auto id1 = ScopedStreamManipulator<TestContext>::ID;
    auto id2 = ScopedStreamManipulator<TestContext>::ID;
    REQUIRE(id1 == id2);
  }

  TEST_CASE("works_with_different_stream_instances") {
    auto stream1 = std::stringstream();
    auto stream2 = std::stringstream();
    auto context1 = TestContext(111);
    auto context2 = TestContext(222);
    auto manipulator1 = ScopedStreamManipulator(stream1, context1);
    auto manipulator2 = ScopedStreamManipulator(stream2, context2);
    auto retrieved1 = get_context<TestContext>(stream1);
    auto retrieved2 = get_context<TestContext>(stream2);
    REQUIRE(retrieved1->m_value == 111);
    REQUIRE(retrieved2->m_value == 222);
  }

  TEST_CASE("modifying_original_context_affects_retrieved_context") {
    auto stream = std::stringstream();
    auto context = TestContext(50);
    auto manipulator = ScopedStreamManipulator(stream, context);
    context.m_value = 75;
    auto retrieved = get_context<TestContext>(stream);
    REQUIRE(retrieved->m_value == 75);
  }

  TEST_CASE("multiple_sequential_scopes") {
    auto stream = std::stringstream();
    {
      auto context1 = TestContext(1);
      auto manipulator1 = ScopedStreamManipulator(stream, context1);
      auto retrieved = get_context<TestContext>(stream);
      REQUIRE(retrieved->m_value == 1);
    }
    {
      auto context2 = TestContext(2);
      auto manipulator2 = ScopedStreamManipulator(stream, context2);
      auto retrieved = get_context<TestContext>(stream);
      REQUIRE(retrieved->m_value == 2);
    }
    {
      auto context3 = TestContext(3);
      auto manipulator3 = ScopedStreamManipulator(stream, context3);
      auto retrieved = get_context<TestContext>(stream);
      REQUIRE(retrieved->m_value == 3);
    }
    auto retrieved = get_context<TestContext>(stream);
    REQUIRE(!retrieved);
  }

  TEST_CASE("works_with_std_cout") {
    auto context = TestContext(999);
    auto manipulator = ScopedStreamManipulator(std::cout, context);
    auto retrieved = get_context<TestContext>(std::cout);
    REQUIRE(retrieved);
    REQUIRE(retrieved->m_value == 999);
  }
}
