#ifndef BEAM_SHUTTLE_DECIMAL_HPP
#define BEAM_SHUTTLE_DECIMAL_HPP
#include <sstream>
#include <boost/multiprecision/cpp_dec_float.hpp>
#include "Beam/Serialization/Receiver.hpp"
#include "Beam/Serialization/Sender.hpp"

namespace Beam {
  template<unsigned Digits10, typename ExponentType, typename Allocator>
  constexpr auto is_structure<
    boost::multiprecision::number<boost::multiprecision::cpp_dec_float<
      Digits10, ExponentType, Allocator>>> = false;

  template<unsigned Digits10, typename ExponentType, typename Allocator>
  struct Send<
      boost::multiprecision::number<boost::multiprecision::cpp_dec_float<
        Digits10, ExponentType, Allocator>>> {
    template<IsSender S>
    void operator ()(S& sender, const char* name,
        const boost::multiprecision::number<
          boost::multiprecision::cpp_dec_float<
            Digits10, ExponentType, Allocator>>& value) const {
      using number_type = boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<
          Digits10, ExponentType, Allocator>>;
      auto digits = static_cast<std::streamsize>(
        std::numeric_limits<number_type>::digits10) + 1;
      auto text = value.str(digits, std::ios_base::fmtflags(0));
      sender.send(name, text);
    }
  };

  template<unsigned Digits10, typename ExponentType, typename Allocator>
  struct Receive<
      boost::multiprecision::number<boost::multiprecision::cpp_dec_float<
        Digits10, ExponentType, Allocator>>> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name,
        boost::multiprecision::number<
          boost::multiprecision::cpp_dec_float<
            Digits10, ExponentType, Allocator>>& value) const {
      value = boost::multiprecision::number<
        boost::multiprecision::cpp_dec_float<Digits10, ExponentType,
          Allocator>>(receive<std::string>(receiver, name));
    }
  };
}

#endif
