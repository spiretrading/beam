#ifndef BEAM_SHUTTLE_UNORDERED_MAP_HPP
#define BEAM_SHUTTLE_UNORDERED_MAP_HPP
#include <unordered_map>
#include "Beam/Serialization/ShuttlePair.hpp"

namespace Beam {
  template<typename K, typename T, typename H, typename P, typename A>
  constexpr auto is_structure<std::unordered_map<K, T, H, P, A>> = false;

  template<typename K, typename T, typename H, typename P, typename A>
  struct Send<std::unordered_map<K, T, H, P, A>> {
    template<IsSender S>
    void operator ()(S& sender, const char* name,
        const std::unordered_map<K, T, H, P, A>& value) const {
      sender.start_sequence(name, static_cast<int>(value.size()));
      for(auto& i : value) {
        sender.send(i);
      }
      sender.end_sequence();
    }
  };

  template<typename K, typename T, typename H, typename P, typename A>
  struct Receive<std::unordered_map<K, T, H, P, A>> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name,
        std::unordered_map<K, T, H, P, A>& value) const {
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
