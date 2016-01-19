#ifndef BEAM_REVERSEDECODER_HPP
#define BEAM_REVERSEDECODER_HPP
#include <algorithm>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/CodecsTests/CodecsTests.hpp"

namespace Beam {
namespace Codecs {
namespace Tests {

  /*! \class ReverseDecoder
      \brief Reverses a data source.
   */
  class ReverseDecoder {
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

  inline std::size_t ReverseDecoder::Decode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    if(sourceSize == 0) {
      return 0;
    }
    if(destinationSize < sourceSize) {
      BOOST_THROW_EXCEPTION(DecoderException(
        "The destination was not large enough to hold the decoded data."));
    }
    std::reverse_copy(static_cast<const char*>(source),
      static_cast<const char*>(source) + sourceSize,
      static_cast<char*>(destination));
    return sourceSize;
  }

  template<typename Buffer>
  std::size_t ReverseDecoder::Decode(const Buffer& source, void* destination,
      std::size_t destinationSize) {
    return Decode(source.GetData(), source.GetSize(), destination,
      destinationSize);
  }

  template<typename Buffer>
  std::size_t ReverseDecoder::Decode(const void* source, std::size_t sourceSize,
      Out<Buffer> destination) {
    destination->Reserve(sourceSize);
    return Decode(source, sourceSize, destination->GetMutableData(),
      destination->GetSize());
  }

  template<typename SourceBuffer, typename DestinationBuffer>
  std::size_t ReverseDecoder::Decode(const SourceBuffer& source,
      Out<DestinationBuffer> destination) {
    return Decode(source.GetData(), source.GetSize(), Store(destination));
  }
}
}

  template<>
  struct ImplementsConcept<Codecs::Tests::ReverseDecoder, Codecs::Decoder> :
    std::true_type {};
}

#endif
