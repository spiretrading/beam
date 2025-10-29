#ifndef BEAM_SHUTTLE_PAIR_HPP
#define BEAM_SHUTTLE_PAIR_HPP
#include <utility>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<typename T1, typename T2>
  constexpr auto is_structure<std::pair<T1, T2>> = false;

  template<typename T1, typename T2>
  struct Send<std::pair<T1, T2>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::pair<T1, T2>& value) const {
      sender.start_sequence(name);
      sender.send(value.first);
      sender.send(value.second);
      sender.end_sequence();
    }
  };

  template<typename T1, typename T2>
  struct Receive<std::pair<T1, T2>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::pair<T1, T2>& value) const {
      receiver.start_sequence(name);
      receiver.receive(value.first);
      receiver.receive(value.second);
      receiver.end_sequence();
    }
  };
}

#endif
