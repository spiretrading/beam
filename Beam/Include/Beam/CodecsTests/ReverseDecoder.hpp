#ifndef BEAM_REVERSE_DECODER_HPP
#define BEAM_REVERSE_DECODER_HPP
#include <algorithm>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/DecoderException.hpp"

namespace Beam::Tests {
  class ReverseEncoder;

  /** Reverses a data source. */
  class ReverseDecoder {
    public:
      template<IsConstBuffer S, IsBuffer T>
      std::size_t decode(const S& source, Out<T> destination);
  };

  template<IsConstBuffer S, IsBuffer T>
  std::size_t ReverseDecoder::decode(const S& source, Out<T> destination) {
    auto available_size = reserve(*destination, source.get_size());
    if(available_size < source.get_size()) {
      boost::throw_with_location(DecoderException(
        "The destination was not large enough to hold the decoded data."));
    }
    std::reverse_copy(source.get_data(), source.get_data() + source.get_size(),
      destination->get_mutable_data());
    return source.get_size();
  }
}

namespace Beam {
  template<>
  struct inverse<Tests::ReverseDecoder> {
    using type = Tests::ReverseEncoder;
  };
}

#endif
