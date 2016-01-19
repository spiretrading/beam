#ifndef BEAM_NULLENCODER_HPP
#define BEAM_NULLENCODER_HPP
#include <cstring>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Codecs/EncoderException.hpp"
#include "Beam/IO/Buffer.hpp"

namespace Beam {
namespace Codecs {

  /*! \class NullEncoder
      \brief An Encoder that leaves its contents 'as-is'.
   */
  class NullEncoder {
    public:
      std::size_t Encode(const void* source, std::size_t sourceSize,
        void* destination, std::size_t destinationSize);

      template<typename Buffer>
      std::size_t Encode(const Buffer& source, void* destination,
        std::size_t destinationSize);

      template<typename Buffer>
      std::size_t Encode(const void* source, std::size_t sourceSize,
        Out<Buffer> destination);

      template<typename SourceBuffer, typename DestinationBuffer>
      std::size_t Encode(const SourceBuffer& source,
        Out<DestinationBuffer> destination);
  };

  template<>
  struct InPlaceSupport<NullEncoder> : std::true_type {};

  template<>
  struct Inverse<NullEncoder> {
    using type = NullDecoder;
  };

  inline std::size_t NullEncoder::Encode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    if(source == destination) {
      return sourceSize;
    }
    if(destinationSize < sourceSize) {
      BOOST_THROW_EXCEPTION(EncoderException(
        "The destination was not large enough to hold the encoded data."));
    }
    std::memcpy(destination, source, sourceSize);
    return sourceSize;
  }

  template<typename Buffer>
  std::size_t NullEncoder::Encode(const Buffer& source, void* destination,
      std::size_t destinationSize) {
    return Encode(source.GetData(), source.GetSize(), destination,
      destinationSize);
  }

  template<typename Buffer>
  std::size_t NullEncoder::Encode(const void* source, std::size_t sourceSize,
      Out<Buffer> destination) {
    destination->Reserve(sourceSize);
    return Encode(source, sourceSize, destination->GetMutableData(),
      destination->GetSize());
  }

  template<typename SourceBuffer, typename DestinationBuffer>
  std::size_t NullEncoder::Encode(const SourceBuffer& source,
      Out<DestinationBuffer> destination) {
    return Encode(source.GetData(), source.GetSize(), Store(destination));
  }
}

  template<>
  struct ImplementsConcept<Codecs::NullEncoder, Codecs::Encoder> :
    std::true_type {};
}

#endif
