#ifndef BEAM_SHUTTLEDEQUE_HPP
#define BEAM_SHUTTLEDEQUE_HPP
#include <deque>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<typename T, typename A>
  struct IsStructure<std::deque<T, A>> : std::false_type {};

  template<typename T, typename A>
  struct Send<std::deque<T, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::deque<T, A>& value) const {
      shuttle.StartSequence(name, static_cast<int>(value.size()));
      for(auto& i : value) {
        shuttle.Shuttle(i);
      }
      shuttle.EndSequence();
    }
  };

  template<typename T, typename A>
  struct Receive<std::deque<T, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::deque<T, A>& value) const {
      value.clear();
      int size;
      shuttle.StartSequence(name, size);
      value.resize(size);
      for(auto i = 0; i < size; ++i) {
        shuttle.Shuttle(value[i]);
      }
      shuttle.EndSequence();
    }
  };
}
}

#endif
