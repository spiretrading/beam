#ifndef BEAM_SHUTTLEARRAY_HPP
#define BEAM_SHUTTLEARRAY_HPP
#include <array>
#include <boost/throw_exception.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<typename T, std::size_t N>
  struct IsStructure<std::array<T, N>> : std::false_type {};

  template<typename T, std::size_t N>
  struct Send<std::array<T, N>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const std::array<T, N>& value) const {
      shuttle.StartSequence(name, static_cast<int>(N));
      for(const auto& i : value) {
        shuttle.Shuttle(i);
      }
      shuttle.EndSequence();
    }
  };

  template<typename T, std::size_t N>
  struct Receive<std::array<T, N>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        std::array<T, N>& value) const {
      int size;
      shuttle.StartSequence(name, size);
      if(size != N) {
        BOOST_THROW_EXCEPTION(SerializationException("Array size mismatch."));
      }
      for(int i = 0; i < size; ++i) {
        shuttle.Shuttle(value[i]);
      }
      shuttle.EndSequence();
    }
  };
}
}

#endif
