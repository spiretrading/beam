#ifndef BEAM_NULL_ENCODER_HPP
#define BEAM_NULL_ENCODER_HPP
#include <cstring>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Codecs/EncoderException.hpp"
#include "Beam/IO/Buffer.hpp"

namespace Beam {
  class NullDecoder;

  /** An Encoder that leaves its contents 'as-is'. */
  class NullEncoder {
    public:
      template<IsConstBuffer S, IsBuffer B>
      std::size_t encode(const S& source, Out<B> destination);
  };

  template<>
  struct in_place_support<NullEncoder> : std::true_type {};

  template<>
  struct inverse<NullEncoder> {
    using type = NullDecoder;
  };

  template<IsConstBuffer S, IsBuffer B>
  std::size_t NullEncoder::encode(const S& source, Out<B> destination) {
    if(source.get_data() == destination->get_data()) {
      return source.get_size();
    }
    auto available_size = reserve(*destination, source.get_size());
    if(available_size < source.get_size()) {
      boost::throw_with_location(EncoderException(
        "The destination was not large enough to hold the encoded data."));
    }
    std::memcpy(
      destination->get_mutable_data(), source.get_data(), source.get_size());
    return source.get_size();
  }
}

#endif
