#ifndef BEAM_PARAMETEREXPRESSION_HPP
#define BEAM_PARAMETEREXPRESSION_HPP
#include "Beam/Queries/Expression.hpp"
#include "Beam/Queries/ExpressionVisitor.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/StandardDataTypes.hpp"

namespace Beam {
namespace Queries {

  /*! \class ParameterExpression
      \brief Represents a variable/parameter used in an Expression.
   */
  class ParameterExpression : public VirtualExpression,
      public CloneableMixin<ParameterExpression> {
    public:

      //! Constructs a ParameterExpression.
      /*!
        \param index The parameter's index.
        \param type The parameter's type.
      */
      ParameterExpression(int index, const DataType& type);

      //! Returns the parameter's index.
      int GetIndex() const;

      virtual const DataType& GetType() const;

      virtual void Apply(ExpressionVisitor& visitor) const;

    protected:
      virtual std::ostream& ToStream(std::ostream& out) const;

    private:
      friend struct Beam::Serialization::DataShuttle;
      int m_index;
      DataType m_type;

      ParameterExpression();
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);
  };

  inline ParameterExpression::ParameterExpression(int index,
      const DataType& type)
      : m_index{index},
        m_type{type} {}

  inline int ParameterExpression::GetIndex() const {
    return m_index;
  }

  inline const DataType& ParameterExpression::GetType() const {
    return m_type;
  }

  inline void ParameterExpression::Apply(ExpressionVisitor& visitor) const {
    visitor.Visit(*this);
  }

  inline std::ostream& ParameterExpression::ToStream(std::ostream& out) const {
    return out << "(parameter " << m_index << ")";
  }

  inline ParameterExpression::ParameterExpression()
      : m_type{BoolType()} {}

  template<typename Shuttler>
  void ParameterExpression::Shuttle(Shuttler& shuttle, unsigned int version) {
    VirtualExpression::Shuttle(shuttle, version);
    shuttle.Shuttle("index", m_index);
    shuttle.Shuttle("type", m_type);
  }

  inline void ExpressionVisitor::Visit(const ParameterExpression& expression) {
    Visit(static_cast<const VirtualExpression&>(expression));
  }
}
}

#endif
