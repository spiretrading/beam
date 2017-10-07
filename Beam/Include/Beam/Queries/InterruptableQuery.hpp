#ifndef BEAM_INTERRUPTABLEQUERY_HPP
#define BEAM_INTERRUPTABLEQUERY_HPP
#include <ostream>
#include "Beam/Queries/InterruptionPolicy.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \class InterruptableQuery
      \brief A Query with a policy for recovering from an interruption or
             failure.
   */
  class InterruptableQuery {
    public:

      //! Constructs an InterruptableQuery with a policy to
      //! BREAK when interrupted.
      InterruptableQuery();

      //! Constructs an InterruptableQuery with a specified InterruptionPolicy.
      /*!
        \param policy The InterruptionPolicy to use.
      */
      InterruptableQuery(InterruptionPolicy policy);

      //! Returns the InterruptionPolicy.
      InterruptionPolicy GetInterruptionPolicy() const;

      //! Sets the InterruptionPolicy.
      void SetInterruptionPolicy(InterruptionPolicy policy);

    private:
      friend struct Serialization::Shuttle<InterruptableQuery>;
      InterruptionPolicy m_interruptionPolicy;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const InterruptableQuery& query) {
    return out << query.GetInterruptionPolicy();
  }

  inline InterruptableQuery::InterruptableQuery()
      : m_interruptionPolicy{InterruptionPolicy::BREAK_QUERY} {}

  inline InterruptableQuery::InterruptableQuery(InterruptionPolicy policy)
      : m_interruptionPolicy{policy} {}

  inline InterruptionPolicy InterruptableQuery::GetInterruptionPolicy() const {
    return m_interruptionPolicy;
  }

  inline void InterruptableQuery::SetInterruptionPolicy(
      InterruptionPolicy policy) {
    m_interruptionPolicy = policy;
  }
}
}

namespace Beam {
namespace Serialization {
  template<>
  struct Shuttle<Queries::InterruptableQuery> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Queries::InterruptableQuery& value,
        unsigned int version) {
      shuttle.Shuttle("interruption_policy", value.m_interruptionPolicy);
    }
  };
}
}

#endif
