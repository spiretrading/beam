#include <string>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Queries/TraversalExpressionVisitor.hpp"

using namespace Beam;

namespace {
  struct RecordingVisitor : TraversalExpressionVisitor {
    std::vector<std::string> m_events;

    void visit(const AndExpression& expression) override {
      m_events.push_back("and");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(const ConstantExpression& expression) override {
      m_events.push_back("constant");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(const FunctionExpression& expression) override {
      m_events.push_back("function");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(
        const GlobalVariableDeclarationExpression& expression) override {
      m_events.push_back("global");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(const MemberAccessExpression& expression) override {
      m_events.push_back("member");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(const NotExpression& expression) override {
      m_events.push_back("not");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(const OrExpression& expression) override {
      m_events.push_back("or");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(const ParameterExpression& expression) override {
      m_events.push_back("parameter");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(const ReduceExpression& expression) override {
      m_events.push_back("reduce");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(const SetVariableExpression& expression) override {
      m_events.push_back("set");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(const VariableExpression& expression) override {
      m_events.push_back("variable");
      TraversalExpressionVisitor::visit(expression);
    }

    void visit(const VirtualExpression& expression) override {
      m_events.push_back("virtual");
      TraversalExpressionVisitor::visit(expression);
    }
  };
}

TEST_SUITE("TraversalExpressionVisitor") {
  TEST_CASE("and_traversal") {
    auto expression =
      AndExpression(ConstantExpression(true), ConstantExpression(false));
    auto visitor = RecordingVisitor();
    expression.apply(visitor);
    REQUIRE(visitor.m_events ==
      std::vector<std::string>{"and", "constant", "constant"});
  }

  TEST_CASE("constant_traversal") {
    auto expression = ConstantExpression(5);
    auto visitor = RecordingVisitor();
    expression.apply(visitor);
    REQUIRE(visitor.m_events.size() == 1);
    REQUIRE(visitor.m_events[0] == "constant");
  }

  TEST_CASE("function_traversal") {
    auto f = FunctionExpression("f", typeid(int),
      std::vector<Expression>{ConstantExpression(1), ConstantExpression(2)});
    auto visitor = RecordingVisitor();
    f.apply(visitor);
    REQUIRE(visitor.m_events ==
      std::vector<std::string>{"function", "constant", "constant"});
  }

  TEST_CASE("global_traversal") {
    auto expression = GlobalVariableDeclarationExpression(
      "g", ConstantExpression(9),
        MemberAccessExpression("m", typeid(int), ConstantExpression(7)));
    auto visitor = RecordingVisitor();
    expression.apply(visitor);
    REQUIRE(visitor.m_events ==
      std::vector<std::string>{"global", "constant", "member", "constant"});
  }

  TEST_CASE("member_access_traversal") {
    auto expression =
      MemberAccessExpression("m", typeid(int), ConstantExpression(5));
    auto visitor = RecordingVisitor();
    expression.apply(visitor);
    REQUIRE(visitor.m_events == std::vector<std::string>{"member", "constant"});
  }

  TEST_CASE("not_traversal") {
    auto expression = NotExpression(ConstantExpression(true));
    auto visitor = RecordingVisitor();
    expression.apply(visitor);
    REQUIRE(visitor.m_events == std::vector<std::string>{"not", "constant"});
  }

  TEST_CASE("or_expression_traversal") {
    auto expression =
      OrExpression(ConstantExpression(false), ConstantExpression(true));
    auto visitor = RecordingVisitor();
    expression.apply(visitor);
    REQUIRE(visitor.m_events ==
      std::vector<std::string>{"or", "constant", "constant"});
  }

  TEST_CASE("parameter_traversal") {
    auto expression = ParameterExpression(0, typeid(int));
    auto visitor = RecordingVisitor();
    expression.apply(visitor);
    REQUIRE(visitor.m_events == std::vector<std::string>{"parameter"});
  }

  TEST_CASE("reduce_traversal") {
    auto reducer = FunctionExpression("r", typeid(int),
      std::vector<Expression>{ParameterExpression(0, typeid(int)),
        ParameterExpression(1, typeid(int))});
    auto expression =
      ReduceExpression(reducer, ConstantExpression(0), Value(0));
    auto visitor = RecordingVisitor();
    expression.apply(visitor);
    REQUIRE(visitor.m_events == std::vector<std::string>{
      "reduce", "function", "parameter", "parameter", "constant"});
  }

  TEST_CASE("set_and_variable_traversal") {
    auto set = SetVariableExpression("x", ConstantExpression(11));
    auto variable = VariableExpression("x", typeid(int));
    auto visitor = RecordingVisitor();
    set.apply(visitor);
    REQUIRE(visitor.m_events == std::vector<std::string>{"set", "constant"});
    visitor.m_events.clear();
    variable.apply(visitor);
    REQUIRE(visitor.m_events == std::vector<std::string>{"variable"});
  }

  TEST_CASE("virtual_expression_traversal") {
    struct TestVirtual : VirtualExpression {
      std::type_index get_type() const override {
        return typeid(bool);
      }
      void apply(ExpressionVisitor& visitor) const override {
        visitor.visit(*this);
      }
    };
    auto expression = TestVirtual();
    auto visitor = RecordingVisitor();
    expression.apply(visitor);
    REQUIRE(visitor.m_events == std::vector<std::string>{"virtual"});
  }
}
