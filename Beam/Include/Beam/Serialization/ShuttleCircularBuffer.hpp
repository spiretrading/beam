#ifndef BEAM_SHUTTLECIRCULARBUFFER_HPP
#define BEAM_SHUTTLECIRCULARBUFFER_HPP
#include <boost/circular_buffer.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<typename T, typename A>
  struct IsStructure<boost::circular_buffer<T, A>> : std::false_type {};

  template<typename T, typename A>
  struct Send<boost::circular_buffer<T, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const boost::circular_buffer<T, A>& value) const {
      shuttle.StartSequence(name, static_cast<int>(value.size()));
      for(auto& i : value) {
        shuttle.Shuttle(i);
      }
      shuttle.EndSequence();
    }
  };

  template<typename T, typename A>
  struct Receive<boost::circular_buffer<T, A>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::circular_buffer<T, A>& value) const {
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
