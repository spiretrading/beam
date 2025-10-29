#include <doctest/doctest.h>
#include "Beam/Queries/GlobalVariableDeclarationEvaluatorNode.hpp"

using namespace Beam;

namespace {
  template<typename T>
  struct TestValueEvaluatorNode : EvaluatorNode<T> {
    T m_value;
    int m_count;

    TestValueEvaluatorNode(T value)
      : m_value(std::move(value)),
        m_count(0) {}

    T eval() override {
      ++m_count;
      return m_value;
    }
  };

  template<typename V, typename B>
  struct BodyReadsVariable : EvaluatorNode<B> {
    GlobalVariableDeclarationEvaluatorNode<V, B>* m_parent;
    int m_count;

    BodyReadsVariable(GlobalVariableDeclarationEvaluatorNode<V, B>& parent)
      : m_parent(&parent),
        m_count(0) {}

    B eval() override {
      ++m_count;
      return m_parent->get_variable();
    }
  };
}

TEST_SUITE("GlobalVariableDeclarationEvaluatorNode") {
  TEST_CASE("lazy_initialization_and_body_calls") {
    auto initial = std::make_unique<TestValueEvaluatorNode<int>>(10);
    auto node =
      GlobalVariableDeclarationEvaluatorNode<int, int>(std::move(initial));
    auto body = std::make_unique<BodyReadsVariable<int, int>>(node);
    node.set_body(std::move(body));
    auto first_result = node.eval();
    REQUIRE(first_result == 10);
    auto second_result = node.eval();
    REQUIRE(second_result == 10);
    node.get_variable() = 42;
    auto third_result = node.eval();
    REQUIRE(third_result == 42);
  }

  TEST_CASE("initial_value") {
    auto initial = std::make_unique<TestValueEvaluatorNode<int>>(5);
    auto initial_ptr = initial.get();
    auto node =
      GlobalVariableDeclarationEvaluatorNode<int, int>(std::move(initial));
    auto body = std::make_unique<TestValueEvaluatorNode<int>>(0);
    auto body_ptr = body.get();
    node.set_body(std::move(body));
    REQUIRE(initial_ptr->m_count == 0);
    REQUIRE(body_ptr->m_count == 0);
    auto a = node.eval();
    REQUIRE(a == 0);
    REQUIRE(node.get_variable() == 5);
    REQUIRE(initial_ptr->m_count == 1);
    REQUIRE(body_ptr->m_count == 1);
    auto b = node.eval();
    REQUIRE(b == 0);
    REQUIRE(initial_ptr->m_count == 1);
    REQUIRE(body_ptr->m_count == 2);
  }
}
