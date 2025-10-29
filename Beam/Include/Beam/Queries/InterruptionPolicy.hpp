#ifndef BEAM_INTERRUPTION_POLICY_HPP
#define BEAM_INTERRUPTION_POLICY_HPP
#include <boost/throw_exception.hpp>
#include "Beam/Collections/Enum.hpp"

namespace Beam {

  /** Enumerates the ways to recover from a Query being interrupted. */
  BEAM_ENUM(InterruptionPolicy,

    /** Breaks the Query. */
    BREAK_QUERY,

    /** Recovers all lost data. */
    RECOVER_DATA,

    /** Ignores all lost data. */
    IGNORE_CONTINUE
  );

  template<>
  constexpr auto is_structure<InterruptionPolicy> = false;

  template<>
  struct Send<InterruptionPolicy> {
    template<IsSender S>
    void operator ()(
        S& shuttle, const char* name, InterruptionPolicy value) const {
      shuttle.send(name, static_cast<int>(value));
    }
  };

  template<>
  struct Receive<InterruptionPolicy> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, InterruptionPolicy& value) const {
      value = static_cast<InterruptionPolicy>(receive<int>(receiver, name));
      if(static_cast<int>(value) < InterruptionPolicy::BREAK_QUERY ||
          static_cast<int>(value) > InterruptionPolicy::IGNORE_CONTINUE) {
        value = InterruptionPolicy::BREAK_QUERY;
        boost::throw_with_location(SerializationException("Invalid policy."));
      }
    }
  };
}

#endif
