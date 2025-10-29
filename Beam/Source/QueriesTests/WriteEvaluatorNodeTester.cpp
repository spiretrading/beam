#include <string>
#include <doctest/doctest.h>
#include "Beam/Queries/ConstantEvaluatorNode.hpp"
#include "Beam/Queries/WriteEvaluatorNode.hpp"

using namespace Beam;

namespace {
  struct MutableSourceNode : EvaluatorNode<int> {
    MutableSourceNode(int* source)
      : m_source(source) {}

    int eval() override {
      return *m_source;
    }

    int* m_source;
  };
}

TEST_SUITE("WriteEvaluatorNode") {
  TEST_CASE("write_int_value") {
    auto destination = 0;
    auto value_node = std::make_unique<ConstantEvaluatorNode<int>>(42);
    auto node = WriteEvaluatorNode(&destination, std::move(value_node));
    REQUIRE(node.eval() == 42);
    REQUIRE(destination == 42);
  }

  TEST_CASE("write_updates_on_subsequent_evals") {
    auto source = 1;
    auto destination = 0;
    auto value_node = std::make_unique<MutableSourceNode>(&source);
    auto node = WriteEvaluatorNode(&destination, std::move(value_node));
    REQUIRE(node.eval() == 1);
    REQUIRE(destination == 1);
    source = 7;
    REQUIRE(node.eval() == 7);
    REQUIRE(destination == 7);
  }

  TEST_CASE("write_string_value") {
    auto destination = std::string();
    auto value_node = std::make_unique<ConstantEvaluatorNode<std::string>>(
      std::string("hello"));
    auto node = WriteEvaluatorNode(&destination, std::move(value_node));
    REQUIRE(node.eval() == "hello");
    REQUIRE(destination == "hello");
  }
}
