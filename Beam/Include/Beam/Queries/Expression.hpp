#ifndef BEAM_QUERIES_EXPRESSION_HPP
#define BEAM_QUERIES_EXPRESSION_HPP
#include <ostream>
#include "Beam/Pointers/ClonePtr.hpp"
#include "Beam/Queries/DataType.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Utilities/Streamable.hpp"

namespace Beam::Queries {

  /** Base class for an expression used in a Query. */
  class VirtualExpression : public Streamable, public virtual Cloneable {
    public:
      virtual ~VirtualExpression() = default;

      /** Returns the type that this Expression evaluates to. */
      virtual const DataType& GetType() const = 0;

      /**
       * Applies an ExpressionVisitor to this instance.
       * @param visitor The ExpressionVisitor to apply.
       */
      virtual void Apply(ExpressionVisitor& visitor) const = 0;

    protected:

      /** Constructs an Expression. */
      VirtualExpression() = default;

      /**
       * Copies an Expression.
       * @param expression The Expression to copy.
       */
      VirtualExpression(const VirtualExpression& expression) = default;

      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Serialization::DataShuttle;
  };

  template<typename Shuttler>
  void VirtualExpression::Shuttle(Shuttler& shuttle, unsigned int version) {}
}

#endif
