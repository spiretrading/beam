#ifndef BEAM_SHUTTLERATIONAL_HPP
#define BEAM_SHUTTLERATIONAL_HPP
#include <boost/rational.hpp>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<typename T>
  struct Send<boost::rational<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const boost::rational<T>& value,
        unsigned int version) const {
      shuttle.Shuttle("numerator", value.numerator());
      shuttle.Shuttle("denominator", value.denominator());
    }
  };

  template<typename T>
  struct Receive<boost::rational<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, boost::rational<T>& value,
        unsigned int version) const {
      typename boost::rational<T>::int_type numerator;
      shuttle.Shuttle("numerator", numerator);
      typename boost::rational<T>::int_type denominator;
      shuttle.Shuttle("denominator", denominator);
      value.assign(numerator, denominator);
    }
  };
}
}

#endif
