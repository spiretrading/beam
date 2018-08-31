#ifndef BEAM_QUERY_EXPRESSION_VISITOR_HPP
#define BEAM_QUERY_EXPRESSION_VISITOR_HPP
#include <boost/noncopyable.hpp>
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {

  /** Implements the visitor pattern for Expressions. */
  class ExpressionVisitor : private boost::noncopyable {
    public:
      virtual ~ExpressionVisitor() = default;

      //! Visits a ConstantExpression.
      virtual void Visit(const ConstantExpression& expression);

      //! Visits a FunctionExpression.
      virtual void Visit(const FunctionExpression& expression);

      //! Visits a GlobalVariableDeclarationExpression.
      virtual void Visit(const GlobalVariableDeclarationExpression& expression);

      //! Visits a MemberAccessExpression.
      virtual void Visit(const MemberAccessExpression& expression);

      //! Visits an OrExpression.
      virtual void Visit(const OrExpression& expression);

      //! Visits a ParameterExpression.
      virtual void Visit(const ParameterExpression& expression);

      //! Visits a ReduceExpression.
      virtual void Visit(const ReduceExpression& expression);

      //! Visits a SetVariableExpression.
      virtual void Visit(const SetVariableExpression& expression);

      //! Visits a VariableExpression.
      virtual void Visit(const VariableExpression& expression);

      //! Visits the base class VirtualExpression.
      virtual void Visit(const VirtualExpression& expression);
  };

  inline void ExpressionVisitor::Visit(const VirtualExpression& expression) {}
}

#endif
