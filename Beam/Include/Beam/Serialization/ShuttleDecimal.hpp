#ifndef BEAM_SHUTTLEDECIMAL_HPP
#define BEAM_SHUTTLEDECIMAL_HPP
#include <boost/multiprecision/cpp_dec_float.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
namespace Serialization {
  template<unsigned Digits10, typename ExponentType, typename Allocator>
  struct IsStructure<
    boost::multiprecision::number<boost::multiprecision::cpp_dec_float<
    Digits10, ExponentType, Allocator>>> : std::false_type {};

  template<unsigned Digits10, typename ExponentType, typename Allocator>
  struct Send<boost::multiprecision::number<
      boost::multiprecision::cpp_dec_float<Digits10, ExponentType,
      Allocator>>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        const boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<Digits10, ExponentType,
        Allocator>>& value) const {
      std::stringstream sink;
      sink << value;
      shuttle.Send(name, sink.str());
    }
  };

  template<unsigned Digits10, typename ExponentType, typename Allocator>
  struct Receive<boost::multiprecision::number<
      boost::multiprecision::cpp_dec_float<Digits10, ExponentType,
      Allocator>>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, const char* name,
        boost::multiprecision::number<boost::multiprecision::cpp_dec_float<
        Digits10, ExponentType, Allocator>>& value) const {
      std::string input;
      shuttle.Shuttle(name, input);
      value = boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<Digits10, ExponentType,
        Allocator>>{input};
    }
  };
}
}

#endif
