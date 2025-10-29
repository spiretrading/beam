#ifndef BEAM_SHUTTLE_OPTIONAL_HPP
#define BEAM_SHUTTLE_OPTIONAL_HPP
#include <boost/optional/optional.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<typename T>
  struct Send<boost::optional<T>> {
    template<IsSender S>
    void operator ()(S& sender, const boost::optional<T>& value,
        unsigned int version) const {
      sender.send("is_initialized", value.is_initialized());
      if(value) {
        sender.send("value", *value);
      }
    }
  };

  template<typename T>
  struct Receive<boost::optional<T>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, boost::optional<T>& value, unsigned int version) const {
      if(receive<bool>(receiver, "is_initialized")) {
        value = receive<T>(receiver, "value");
      } else {
        value = boost::none;
      }
    }
  };
}

#endif
