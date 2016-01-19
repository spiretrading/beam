#ifndef BEAM_INTERRUPTIONPOLICY_HPP
#define BEAM_INTERRUPTIONPOLICY_HPP
#include <ostream>
#include <boost/throw_exception.hpp>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/SerializationException.hpp"

namespace Beam {
namespace Queries {

  /*! \enum InterruptionPolicy
      \brief Enumerates the ways to recover from a Query being interrupted.
   */
  enum class InterruptionPolicy {

    //! Breaks the Query.
    BREAK_QUERY,

    //! Recovers all lost data.
    RECOVER_DATA,

    //! Ignores all lost data.
    IGNORE_CONTINUE,
  };

  inline std::ostream& operator <<(std::ostream& out,
      InterruptionPolicy policy) {
    if(policy == InterruptionPolicy::BREAK_QUERY) {
      return out << "BREAK_QUERY";
    } else if(policy == InterruptionPolicy::RECOVER_DATA) {
      return out << "RECOVER_DATA";
    } else if(policy == InterruptionPolicy::IGNORE_CONTINUE) {
      return out << "IGNORE_CONTINUE";
    } else {
      return out << "NONE";
    }
  }
}
}

namespace Beam {
namespace Serialization {
  template<>
  struct IsStructure<Queries::InterruptionPolicy> : std::false_type {};

  template<>
  struct Send<Queries::InterruptionPolicy> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Queries::InterruptionPolicy value) const {
      shuttle.Send(name, value);
    }
  };

  template<>
  struct Receive<Queries::InterruptionPolicy> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        Queries::InterruptionPolicy& value) const {
      shuttle.Shuttle(name, value);
      if(value < Queries::InterruptionPolicy::BREAK_QUERY ||
          value > Queries::InterruptionPolicy::IGNORE_CONTINUE) {
        value = Queries::InterruptionPolicy::BREAK_QUERY;
        BOOST_THROW_EXCEPTION(SerializationException("Invalid policy."));
      }
    }
  };
}
}

#endif
