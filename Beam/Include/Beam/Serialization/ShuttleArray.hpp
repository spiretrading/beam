#ifndef BEAM_SHUTTLE_ARRAY_HPP
#define BEAM_SHUTTLE_ARRAY_HPP
#include <array>
#include <boost/throw_exception.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<typename T, std::size_t N>
  constexpr auto is_structure<std::array<T, N>> = false;

  template<typename T, std::size_t N>
  struct Send<std::array<T, N>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::array<T, N>& value) const {
      sender.start_sequence(name, static_cast<int>(N));
      for(auto& i : value) {
        sender.send(i);
      }
      sender.end_sequence();
    }
  };

  template<typename T, std::size_t N>
  struct Receive<std::array<T, N>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::array<T, N>& value) const {
      auto size = 0;
      receiver.start_sequence(name, size);
      if(size != N) {
        boost::throw_with_location(
          SerializationException("Array size mismatch."));
      }
      for(auto i = 0; i < size; ++i) {
        receiver.receive(value[i]);
      }
      receiver.end_sequence();
    }
  };
}

#endif
