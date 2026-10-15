#ifndef BEAM_QUERIES_EXPRESSION_HPP
#define BEAM_QUERIES_EXPRESSION_HPP
#include <typeindex>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Utilities/Streamable.hpp"

namespace Beam {
  class ExpressionVisitor;
  class VirtualExpression;

  /** Encapsulates an expression used in a Query. */
  class Expression final : public Streamable {
    public:

      /**
       * Constructs an Expression.
       * @param expression The VirtualExpression to encapsulate.
       */
      template<std::derived_from<VirtualExpression> T>
      Expression(T&& value);

      /**
       * Constructs an Expression.
       * @param expression The VirtualExpression to encapsulate.
       */
      template<std::derived_from<VirtualExpression> T>
      Expression(const T& value);

      /**
       * Casts the underlying expression to the given type.
       * @tparam T The type to cast to.
       * @return The underlying expression cast to the given type.
       */
      template<typename T>
      const T& as() const;

      /** Returns the type that this Expression evaluates to. */
      std::type_index get_type() const;

      /**
       * Applies an ExpressionVisitor to this instance.
       * @param visitor The ExpressionVisitor to apply.
       */
      void apply(ExpressionVisitor& visitor) const;

      std::ostream& to_stream(std::ostream& out) const override;

    protected:
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      friend struct DataShuttle;
      std::shared_ptr<VirtualExpression> m_expression;

      Expression() = default;
  };

  /** Base class for an expression used in a Query. */
  class VirtualExpression : public Streamable {
    public:

      /** Returns the type that this Expression evaluates to. */
      virtual std::type_index get_type() const = 0;

      /**
       * Applies an ExpressionVisitor to this instance.
       * @param visitor The ExpressionVisitor to apply.
       */
      virtual void apply(ExpressionVisitor& visitor) const = 0;

    protected:

      /** Constructs a VirtualExpression. */
      VirtualExpression() = default;

      VirtualExpression(const VirtualExpression&) = default;
      VirtualExpression& operator =(const VirtualExpression&) = default;

      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);
  };

  template<std::derived_from<VirtualExpression> T>
  Expression::Expression(T&& value)
    : m_expression(std::make_shared<T>(std::forward<T>(value))) {}

  template<std::derived_from<VirtualExpression> T>
  Expression::Expression(const T& value)
    : m_expression(std::make_shared<T>(value)) {}

  template<typename T>
  const T& Expression::as() const {
    return *static_cast<const T*>(m_expression.get());
  }

  inline std::type_index Expression::get_type() const {
    return m_expression->get_type();
  }

  inline void Expression::apply(ExpressionVisitor& visitor) const {
    m_expression->apply(visitor);
  }

  inline std::ostream& Expression::to_stream(std::ostream& out) const {
    return out << *m_expression;
  }

  template<IsShuttle S>
  void Expression::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("expression", m_expression);
  }

  template<IsShuttle S>
  void VirtualExpression::shuttle(S& shuttle, unsigned int version) {}
}

#endif
