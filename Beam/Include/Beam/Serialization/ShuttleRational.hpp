#ifndef BEAM_SHUTTLE_RATIONAL_HPP
#define BEAM_SHUTTLE_RATIONAL_HPP
#include <boost/rational.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<typename T>
  struct Send<boost::rational<T>> {
    template<IsSender S>
    void operator ()(S& sender, const boost::rational<T>& value,
        unsigned int version) const {
      sender.send("numerator", value.numerator());
      sender.send("denominator", value.denominator());
    }
  };

  template<typename T>
  struct Receive<boost::rational<T>> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, boost::rational<T>& value, unsigned int version) const {
      auto numerator =
        receive<typename boost::rational<T>::int_type>(receiver, "numerator");
      auto denominator = 
        receive<typename boost::rational<T>::int_type>(receiver, "denominator");
      value.assign(std::move(numerator), std::move(denominator));
    }
  };
}

#endif
