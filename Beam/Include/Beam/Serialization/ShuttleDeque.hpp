#ifndef BEAM_SHUTTLE_DEQUE_HPP
#define BEAM_SHUTTLE_DEQUE_HPP
#include <deque>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"
#include "Beam/Serialization/SerializedValue.hpp"

namespace Beam {
  template<typename T, typename A>
  constexpr auto is_structure<std::deque<T, A>> = false;

  template<typename T, typename A>
  struct Send<std::deque<T, A>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::deque<T, A>& value) const {
      sender.start_sequence(name, static_cast<int>(value.size()));
      for(auto& i : value) {
        sender.send(i);
      }
      sender.end_sequence();
    }
  };

  template<typename T, typename A>
  struct Receive<std::deque<T, A>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::deque<T, A>& value) const {
      value.clear();
      auto size = int();
      receiver.start_sequence(name, size);
      for(auto i = 0; i < size; ++i) {
        value.push_back(receive<T>(receiver));
      }
      receiver.end_sequence();
    }
  };
}

#endif
