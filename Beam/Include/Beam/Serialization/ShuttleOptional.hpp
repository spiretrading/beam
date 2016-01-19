#ifndef BEAM_SHUTTLEOPTIONAL_HPP
#define BEAM_SHUTTLEOPTIONAL_HPP
#include <boost/optional/optional.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<typename T>
  struct Send<boost::optional<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const boost::optional<T>& value,
        unsigned int version) const {
      shuttle.Shuttle("is_initialized", value.is_initialized());
      if(value.is_initialized()) {
        shuttle.Shuttle("value", *value);
      }
    }
  };

  template<typename T>
  struct Receive<boost::optional<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, boost::optional<T>& value,
        unsigned int version) const {
      bool isInitialized;
      shuttle.Shuttle("is_initialized", isInitialized);
      if(isInitialized) {
        SerializedValue<T> held;
        shuttle.Shuttle("value", held);
        value = *held;
      } else {
        value = boost::optional<T>();
      }
    }
  };
}
}

#endif
