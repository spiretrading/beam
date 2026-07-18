module;
#include "Prelude.hpp"

export module Beam:ReverseEncoder;

namespace Beam::Tests {
  class ReverseDecoder;

  /** Reverses a data source. */
  class ReverseEncoder {
    public:
      template<IsConstBuffer S, IsBuffer B>
      std::size_t encode(const S& source, Out<B> destination);
  };

  template<IsConstBuffer S, IsBuffer B>
  std::size_t ReverseEncoder::encode(const S& source, Out<B> destination) {
    auto length = source.get_size();
    if(length == 0) {
      return 0;
    }
    auto available_size = reserve(*destination, length);
    if(available_size < length) {
      boost::throw_with_location(EncoderException(
        "The destination was not large enough to hold the encoded data."));
    }
    auto i = source.get_data() + length - 1;
    auto j = destination->get_mutable_data();
    while(length > 0) {
      *j = *i;
      ++j;
      --i;
      --length;
    }
    return available_size;
  }
}

export namespace Beam {
  template<>
  struct inverse<Tests::ReverseEncoder> {
    using type = Tests::ReverseDecoder;
  };
}

