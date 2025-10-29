#ifndef BEAM_TRAVERSAL_EXPRESSION_VISITOR_HPP
#define BEAM_TRAVERSAL_EXPRESSION_VISITOR_HPP
#include "Beam/Queries/AndExpression.hpp"
#include "Beam/Queries/ConstantExpression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/FunctionExpression.hpp"
#include "Beam/Queries/GlobalVariableDeclarationExpression.hpp"
#include "Beam/Queries/MemberAccessExpression.hpp"
#include "Beam/Queries/NotExpression.hpp"
#include "Beam/Queries/OrExpression.hpp"
#include "Beam/Queries/ParameterExpression.hpp"
#include "Beam/Queries/ReduceExpression.hpp"
#include "Beam/Queries/SetVariableExpression.hpp"
#include "Beam/Queries/VariableExpression.hpp"

namespace Beam {

  /** An ExpressionVisitor that traverses all of its children. */
  class TraversalExpressionVisitor : public ExpressionVisitor {
    public:
      void visit(const AndExpression& expression) override;
      void visit(const ConstantExpression& expression) override;
      void visit(const FunctionExpression& expression) override;
      void visit(
        const GlobalVariableDeclarationExpression& expression) override;
      void visit(const MemberAccessExpression& expression) override;
      void visit(const NotExpression& expression) override;
      void visit(const OrExpression& expression) override;
      void visit(const ParameterExpression& expression) override;
      void visit(const ReduceExpression& expression) override;
      void visit(const SetVariableExpression& expression) override;
      void visit(const VariableExpression& expression) override;
      void visit(const VirtualExpression& expression) override;
  };

  inline void TraversalExpressionVisitor::visit(
      const AndExpression& expression) {
    expression.get_left().apply(*this);
    expression.get_right().apply(*this);
  }

  inline void TraversalExpressionVisitor::visit(
    const ConstantExpression& expression) {}

  inline void TraversalExpressionVisitor::visit(
      const FunctionExpression& expression) {
    for(auto& parameter : expression.get_parameters()) {
      parameter.apply(*this);
    }
  }

  inline void TraversalExpressionVisitor::visit(
      const GlobalVariableDeclarationExpression& expression) {
    expression.get_initial_value().apply(*this);
    expression.get_body().apply(*this);
  }

  inline void TraversalExpressionVisitor::visit(
      const MemberAccessExpression& expression) {
    expression.get_expression().apply(*this);
  }

  inline void TraversalExpressionVisitor::visit(
      const NotExpression& expression) {
    expression.get_operand().apply(*this);
  }

  inline void TraversalExpressionVisitor::visit(
      const OrExpression& expression) {
    expression.get_left().apply(*this);
    expression.get_right().apply(*this);
  }

  inline void TraversalExpressionVisitor::visit(
    const ParameterExpression& expression) {}

  inline void TraversalExpressionVisitor::visit(
      const ReduceExpression& expression) {
    expression.get_reducer().apply(*this);
    expression.get_series().apply(*this);
  }

  inline void TraversalExpressionVisitor::visit(
      const SetVariableExpression& expression) {
    expression.get_value().apply(*this);
  }

  inline void TraversalExpressionVisitor::visit(
    const VariableExpression& expression) {}

  inline void TraversalExpressionVisitor::visit(
    const VirtualExpression& expression) {}
}

#endif
