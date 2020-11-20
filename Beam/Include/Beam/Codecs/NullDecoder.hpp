#ifndef BEAM_NULL_DECODER_HPP
#define BEAM_NULL_DECODER_HPP
#include <cstring>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/IO/Buffer.hpp"

namespace Beam {
namespace Codecs {

  /** A Decoder that leaves the data 'as-is'. */
  class NullDecoder {
    public:
      std::size_t Decode(const void* source, std::size_t sourceSize,
        void* destination, std::size_t destinationSize);

      template<typename Buffer>
      std::size_t Decode(const Buffer& source, void* destination,
        std::size_t destinationSize);

      template<typename Buffer>
      std::size_t Decode(const void* source, std::size_t sourceSize,
        Out<Buffer> destination);

      template<typename SourceBuffer, typename DestinationBuffer>
      std::size_t Decode(const SourceBuffer& source,
        Out<DestinationBuffer> destination);
  };

  template<>
  struct InPlaceSupport<NullDecoder> : std::true_type {};

  template<>
  struct Inverse<NullDecoder> {
    using type = NullEncoder;
  };

  inline std::size_t NullDecoder::Decode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    if(source == destination) {
      return sourceSize;
    }
    if(destinationSize < sourceSize) {
      BOOST_THROW_EXCEPTION(DecoderException(
        "The destination was not large enough to hold the decoded data."));
    }
    std::memcpy(destination, source, sourceSize);
    return sourceSize;
  }

  template<typename Buffer>
  std::size_t NullDecoder::Decode(const Buffer& source, void* destination,
      std::size_t destinationSize) {
    return Decode(source.GetData(), source.GetSize(), destination,
      destinationSize);
  }

  template<typename Buffer>
  std::size_t NullDecoder::Decode(const void* source, std::size_t sourceSize,
      Out<Buffer> destination) {
    destination->Reserve(sourceSize);
    return Decode(source, sourceSize, destination->GetMutableData(),
      destination->GetSize());
  }

  template<typename SourceBuffer, typename DestinationBuffer>
  std::size_t NullDecoder::Decode(const SourceBuffer& source,
      Out<DestinationBuffer> destination) {
    return Decode(source.GetData(), source.GetSize(), Store(destination));
  }
}

  template<>
  struct ImplementsConcept<Codecs::NullDecoder, Codecs::Decoder> :
    std::true_type {};
}

#endif
