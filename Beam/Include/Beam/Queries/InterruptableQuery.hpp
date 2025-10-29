#ifndef BEAM_INTERRUPTABLE_QUERY_HPP
#define BEAM_INTERRUPTABLE_QUERY_HPP
#include <ostream>
#include "Beam/Queries/InterruptionPolicy.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /** A Query with a policy for recovering from an interruption or failure. */
  class InterruptableQuery {
    public:

      /**
       * Constructs an InterruptableQuery with a policy to BREAK when
       * interrupted.
       */
      InterruptableQuery() noexcept;

      /**
       * Constructs an InterruptableQuery with a specified InterruptionPolicy.
       * @param policy The InterruptionPolicy to use.
       */
      explicit InterruptableQuery(InterruptionPolicy policy);

      /** Returns the InterruptionPolicy. */
      InterruptionPolicy get_interruption_policy() const;

      /** Sets the InterruptionPolicy. */
      void set_interruption_policy(InterruptionPolicy policy);

      bool operator ==(const InterruptableQuery&) const = default;

    private:
      friend struct Shuttle<InterruptableQuery>;
      InterruptionPolicy m_interruption_policy;
  };

  inline std::ostream& operator <<(
      std::ostream& out, const InterruptableQuery& query) {
    return out << query.get_interruption_policy();
  }

  inline InterruptableQuery::InterruptableQuery() noexcept
    : InterruptableQuery(InterruptionPolicy::BREAK_QUERY) {}

  inline InterruptableQuery::InterruptableQuery(InterruptionPolicy policy)
    : m_interruption_policy(policy) {}

  inline InterruptionPolicy
      InterruptableQuery::get_interruption_policy() const {
    return m_interruption_policy;
  }

  inline void InterruptableQuery::set_interruption_policy(
      InterruptionPolicy policy) {
    m_interruption_policy = policy;
  }

  template<>
  struct Shuttle<InterruptableQuery> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, InterruptableQuery& value, unsigned int version) const {
      shuttle.shuttle("interruption_policy", value.m_interruption_policy);
    }
  };
}

#endif
