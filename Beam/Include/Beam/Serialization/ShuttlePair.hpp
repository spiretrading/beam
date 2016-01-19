#ifndef BEAM_SHUTTLEPAIR_HPP
#define BEAM_SHUTTLEPAIR_HPP
#include <utility>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<typename T1, typename T2>
  struct IsStructure<std::pair<T1, T2>> : std::false_type {};

  template<typename T1, typename T2>
  struct Send<std::pair<T1, T2>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::pair<T1, T2>& value) const {
      shuttle.StartSequence(name);
      shuttle.Shuttle(value.first);
      shuttle.Shuttle(value.second);
      shuttle.EndSequence();
    }
  };

  template<typename T1, typename T2>
  struct Receive<std::pair<T1, T2>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::pair<T1, T2>& value) const {
      shuttle.StartSequence(name);
      shuttle.Shuttle(value.first);
      shuttle.Shuttle(value.second);
      shuttle.EndSequence();
    }
  };
}
}

#endif
