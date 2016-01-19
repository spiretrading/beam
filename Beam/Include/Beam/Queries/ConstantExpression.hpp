#ifndef BEAM_QUERYCONSTANTEXPRESSION_HPP
#define BEAM_QUERYCONSTANTEXPRESSION_HPP
#include <utility>
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/NativeValue.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/Value.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \class ConstantExpression
      \brief An Expression that evaluates to a constant.
   */
  class ConstantExpression : public VirtualExpression,
      public CloneableMixin<ConstantExpression> {
    public:

      //! Constructs a ConstantExpression.
      /*!
        \param value The value to evaluate to.
      */
      ConstantExpression(const Value& value);

      //! Copies a ConstantExpression.
      /*!
        \param expression The ConstantExpression to copy.
      */
      ConstantExpression(const ConstantExpression& expression) = default;

      //! Returns the Value to evaluate to.
      const Value& GetValue() const;

      virtual const DataType& GetType() const;

      virtual void Apply(ExpressionVisitor& visitor) const;

    protected:
      virtual std::ostream& ToStream(std::ostream& out) const;

    private:
      friend struct Serialization::DataShuttle;
      Value m_value;

      ConstantExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  //! Builds a ConstantExpression representing a value.
  /*!
    \param value The value the ConstantExpression will evaluate to.
  */
  template<typename T>
  ConstantExpression MakeConstantExpression(T&& value) {
    return ConstantExpression(MakeNativeValue(std::forward<T>(value)));
  }

  inline ConstantExpression::ConstantExpression(const Value& value)
      : m_value(value) {}

  inline const Value& ConstantExpression::GetValue() const {
    return m_value;
  }

  inline const DataType& ConstantExpression::GetType() const {
    return m_value->GetType();
  }

  inline void ConstantExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& ConstantExpression::ToStream(std::ostream& out) const {
    return out << *m_value;
  }

  inline ConstantExpression::ConstantExpression()
      : m_value(MakeNativeValue(0)) {}

  template<typename Shuttler>
  void ConstantExpression::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("value", m_value);
  }

  inline void ExpressionVisitor::Visit(const ConstantExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}
}

#endif
