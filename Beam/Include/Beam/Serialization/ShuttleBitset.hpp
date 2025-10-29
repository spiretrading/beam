#ifndef BEAM_SHUTTLE_BITSET_HPP
#define BEAM_SHUTTLE_BITSET_HPP
#include <bitset>
#include <cstdint>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<size_t N>
  constexpr auto is_structure<std::bitset<N>> = false;

  template<size_t N>
  struct Send<std::bitset<N>> {
    template<IsSender S>
    void operator ()(S& sender, const char* name, std::bitset<N> value) const {
      sender.send(name, static_cast<std::uint64_t>(value.to_ullong()));
    }
  };

  template<size_t N>
  struct Receive<std::bitset<N>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::bitset<N>& value) const {
      value = std::bitset<N>(static_cast<unsigned long long>(
        receive<std::uint64_t>(receiver, name)));
    }
  };
}

#endif
