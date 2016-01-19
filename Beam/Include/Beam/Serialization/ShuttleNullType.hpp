#ifndef BEAM_SHUTTLENULLTYPE_HPP
#define BEAM_SHUTTLENULLTYPE_HPP
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Utilities/NullType.hpp"

namespace Beam {
namespace Serialization {
  template<>
  struct IsStructure<NullType> : std::false_type {};

  template<>
  struct Send<NullType> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
      const NullType& value) const {}
  };

  template<>
  struct Receive<NullType> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
      NullType& value) const {}
  };
}
}

#endif
