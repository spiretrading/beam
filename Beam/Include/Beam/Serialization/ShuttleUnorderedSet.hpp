#ifndef BEAM_SHUTTLEUNORDEREDSET_HPP
#define BEAM_SHUTTLEUNORDEREDSET_HPP
#include <unordered_set>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<typename K, typename H, typename P, typename A>
  struct IsStructure<std::unordered_set<K, H, P, A>> : std::false_type {};

  template<typename K, typename H, typename P, typename A>
  struct Send<std::unordered_set<K, H, P, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::unordered_set<K, H, P, A>& value) const {
      shuttle.StartSequence(name, static_cast<int>(value.size()));
      for(auto& i : value) {
        shuttle.Shuttle(i);
      }
      shuttle.EndSequence();
    }
  };

  template<typename K, typename H, typename P, typename A>
  struct Receive<std::unordered_set<K, H, P, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::unordered_set<K, H, P, A>& value) const {
      value.clear();
      int size;
      shuttle.StartSequence(name, size);
      for(auto i = 0; i < size; ++i) {
        K entry;
        shuttle.Shuttle(entry);
        value.insert(entry);
      }
      shuttle.EndSequence();
    }
  };
}
}

#endif
