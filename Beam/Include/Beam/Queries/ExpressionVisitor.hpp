#ifndef BEAM_QUERY_EXPRESSION_VISITOR_HPP
#define BEAM_QUERY_EXPRESSION_VISITOR_HPP

namespace Beam {
  class AndExpression;
  class ConstantExpression;
  class FunctionExpression;
  class GlobalVariableDeclarationExpression;
  class MemberAccessExpression;
  class NotExpression;
  class OrExpression;
  class ParameterExpression;
  class ReduceExpression;
  class SetVariableExpression;
  class VariableExpression;
  class VirtualExpression;

  /** Implements the visitor pattern for Expressions. */
  class ExpressionVisitor {
    public:
      virtual ~ExpressionVisitor() = default;

      /** Visits an AndExpression. */
      virtual void visit(const AndExpression& expression);

      /** Visits a ConstantExpression. */
      virtual void visit(const ConstantExpression& expression);

      /** Visits a FunctionExpression. */
      virtual void visit(const FunctionExpression& expression);

      /** Visits a GlobalVariableDeclarationExpression. */
      virtual void visit(const GlobalVariableDeclarationExpression& expression);

      /** Visits a MemberAccessExpression. */
      virtual void visit(const MemberAccessExpression& expression);

      /** Visits a NotExpression. */
      virtual void visit(const NotExpression& expression);

      /** Visits an OrExpression. */
      virtual void visit(const OrExpression& expression);

      /** Visits a ParameterExpression. */
      virtual void visit(const ParameterExpression& expression);

      /** Visits a ReduceExpression. */
      virtual void visit(const ReduceExpression& expression);

      /** Visits a SetVariableExpression. */
      virtual void visit(const SetVariableExpression& expression);

      /** Visits a VariableExpression. */
      virtual void visit(const VariableExpression& expression);

      /** Visits the base class VirtualExpression. */
      virtual void visit(const VirtualExpression& expression);

    protected:

      /** Constructs an ExpressionVisitor. */
      ExpressionVisitor() = default;

    private:
      ExpressionVisitor(const ExpressionVisitor&) = delete;
      ExpressionVisitor& operator =(const ExpressionVisitor&) = delete;
  };

  inline void ExpressionVisitor::visit(const VirtualExpression& expression) {}
}

#endif
