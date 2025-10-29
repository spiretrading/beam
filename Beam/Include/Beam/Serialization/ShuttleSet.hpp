#ifndef BEAM_SHUTTLE_SET_HPP
#define BEAM_SHUTTLE_SET_HPP
#include <set>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<typename K, typename C, typename A>
  constexpr auto is_structure<std::set<K, C, A>> = false;

  template<typename K, typename C, typename A>
  struct Send<std::set<K, C, A>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::set<K, C, A>& value) const {
      sender.start_sequence(name, static_cast<int>(value.size()));
      for(auto& i : value) {
        sender.send(i);
      }
      sender.end_sequence();
    }
  };

  template<typename K, typename C, typename A>
  struct Receive<std::set<K, C, A>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::set<K, C, A>& value) const {
      value.clear();
      auto size = int();
      receiver.start_sequence(name, size);
      for(auto i = 0; i < size; ++i) {
        value.insert(receive<K>(receiver));
      }
      receiver.end_sequence();
    }
  };
}

#endif
