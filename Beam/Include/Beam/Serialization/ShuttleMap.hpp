#ifndef BEAM_SHUTTLEMAP_HPP
#define BEAM_SHUTTLEMAP_HPP
#include <map>
#include "Beam/Serialization/ShuttlePair.hpp"

namespace Beam {
namespace Serialization {
  template<typename K, typename T, typename C, typename A>
  struct IsStructure<std::map<K, T, C, A>> : std::false_type {};

  template<typename K, typename T, typename C, typename A>
  struct Send<std::map<K, T, C, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::map<K, T, C, A>& value) const {
      shuttle.StartSequence(name, static_cast<int>(value.size()));
      for(const auto& i : value) {
        shuttle.Shuttle(i);
      }
      shuttle.EndSequence();
    }
  };

  template<typename K, typename T, typename C, typename A>
  struct Receive<std::map<K, T, C, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::map<K, T, C, A>& value) const {
      int size;
      shuttle.StartSequence(name, size);
      for(int i = 0; i < size; ++i) {
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
