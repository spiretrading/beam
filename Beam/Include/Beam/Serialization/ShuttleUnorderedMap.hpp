#ifndef BEAM_SHUTTLEUNORDEREDMAP_HPP
#define BEAM_SHUTTLEUNORDEREDMAP_HPP
#include <unordered_map>
#include "Beam/Serialization/ShuttlePair.hpp"

namespace Beam {
namespace Serialization {
  template<typename K, typename T, typename H, typename P, typename A>
  struct IsStructure<std::unordered_map<K, T, H, P, A>> : std::false_type {};

  template<typename K, typename T, typename H, typename P, typename A>
  struct Send<std::unordered_map<K, T, H, P, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::unordered_map<K, T, H, P, A>& value) const {
      shuttle.StartSequence(name, static_cast<int>(value.size()));
      for(auto& i : value) {
        shuttle.Shuttle(i);
      }
      shuttle.EndSequence();
    }
  };

  template<typename K, typename T, typename H, typename P, typename A>
  struct Receive<std::unordered_map<K, T, H, P, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::unordered_map<K, T, H, P, A>& value) const {
      value.clear();
      int size;
      shuttle.StartSequence(name, size);
      for(auto i = 0; i < size; ++i) {
        std::pair<K, T> entry;
        shuttle.Shuttle(entry);
        value.insert(entry);
      }
      shuttle.EndSequence();
    }
  };
}
}

#endif
