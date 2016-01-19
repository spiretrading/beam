#ifndef BEAM_REVERSEENCODER_HPP
#define BEAM_REVERSEENCODER_HPP
#include <algorithm>
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/CodecsTests/CodecsTests.hpp"

namespace Beam {
namespace Codecs {
namespace Tests {

  /*! \class ReverseEncoder
      \brief Reverses a data source.
   */
  class ReverseEncoder {
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

  inline std::size_t ReverseEncoder::Encode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    auto length = std::min(sourceSize, destinationSize);
    auto size = length;
    auto sourceIterator = static_cast<const char*>(source) + length - 1;
    auto destinationIterator = static_cast<char*>(destination);
    while(length > 0) {
      *destinationIterator = *sourceIterator;
      ++destinationIterator;
      --sourceIterator;
      --length;
    }
    return size;
  }

  template<typename Buffer>
  std::size_t ReverseEncoder::Encode(const Buffer& source, void* destination,
      std::size_t destinationSize) {
    return Encode(source.GetData(), source.GetSize(), destination,
      destinationSize);
  }

  template<typename Buffer>
  std::size_t ReverseEncoder::Encode(const void* source, std::size_t sourceSize,
      Out<Buffer> destination) {
    auto startingPoint = destination->GetSize();
    destination->Grow(sourceSize);
    return Encode(source, sourceSize,
      destination->GetMutableData() + startingPoint, sourceSize);
  }

  template<typename SourceBuffer, typename DestinationBuffer>
  std::size_t ReverseEncoder::Encode(const SourceBuffer& source,
      Out<DestinationBuffer> destination) {
    return Encode(source.GetData(), source.GetSize(), destination);
  }
}

  template<>
  struct Inverse<Tests::ReverseEncoder> {
    using type = Tests::ReverseDecoder;
  };
}

  template<>
  struct ImplementsConcept<Codecs::Tests::ReverseEncoder, Codecs::Encoder> :
    std::true_type {};
}

#endif
