#ifndef BEAM_SHUTTLEBITSET_HPP
#define BEAM_SHUTTLEBITSET_HPP
#include <bitset>
#include <cstdint>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<size_t N>
  struct IsStructure<std::bitset<N>> : std::false_type {};

  template<size_t N>
  struct Send<std::bitset<N>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::bitset<N> value) const {
      std::uint64_t v = static_cast<std::uint64_t>(value.to_ullong());
      shuttle.Send(name, v);
    }
  };

  template<size_t N>
  struct Receive<std::bitset<N>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::bitset<N>& value) const {
      std::uint64_t v;
      shuttle.Shuttle(name, v);
      std::bitset<N> b(static_cast<unsigned long long>(v));
      value = b;
    }
  };
}
}

#endif
