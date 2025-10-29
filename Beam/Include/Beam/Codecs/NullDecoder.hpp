#ifndef BEAM_NULL_DECODER_HPP
#define BEAM_NULL_DECODER_HPP
#include <cstring>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/IO/Buffer.hpp"

namespace Beam {
  class NullEncoder;

  /** A Decoder that leaves the data as-is. */
  class NullDecoder {
    public:
      template<IsConstBuffer S, IsBuffer T>
      std::size_t decode(const S& source, Out<T> destination);
  };

  template<>
  struct in_place_support<NullDecoder> : std::true_type {};

  template<>
  struct inverse<NullDecoder> {
    using type = NullEncoder;
  };

  template<IsConstBuffer S, IsBuffer T>
  std::size_t NullDecoder::decode(const S& source, Out<T> destination) {
    if(source.get_data() == destination->get_data()) {
      return source.get_size();
    }
    auto available_size = reserve(*destination, source.get_size());
    if(available_size < source.get_size()) {
      boost::throw_with_location(DecoderException(
        "The destination was not large enough to hold the decoded data."));
    }
    std::memcpy(
      destination->get_mutable_data(), source.get_data(), source.get_size());
    return source.get_size();
  }
}

#endif
