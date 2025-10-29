#ifndef BEAM_SHUTTLE_MAP_HPP
#define BEAM_SHUTTLE_MAP_HPP
#include <map>
#include "Beam/Serialization/ShuttlePair.hpp"

namespace Beam {
  template<typename K, typename T, typename C, typename A>
  constexpr auto is_structure<std::map<K, T, C, A>> = false;

  template<typename K, typename T, typename C, typename A>
  struct Send<std::map<K, T, C, A>> {
    template<IsSender S>
    void operator ()(
        S& sender, const char* name, const std::map<K, T, C, A>& value) const {
      sender.start_sequence(name, static_cast<int>(value.size()));
      for(auto& i : value) {
        sender.send(i);
      }
      sender.end_sequence();
    }
  };

  template<typename K, typename T, typename C, typename A>
  struct Receive<std::map<K, T, C, A>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, const char* name, std::map<K, T, C, A>& value) const {
      auto size = int();
      receiver.start_sequence(name, size);
      for(auto i = 0; i < size; ++i) {
        value.insert(receive<std::pair<K, T>>(receiver));
      }
      receiver.end_sequence();
    }
  };
}

#endif
