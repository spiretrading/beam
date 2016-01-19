#ifndef BEAM_ZLIBDECODER_HPP
#define BEAM_ZLIBDECODER_HPP
#include <zlib.h>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/IO/Buffer.hpp"

namespace Beam {
namespace Codecs {

  /*! \class ZLibDecoder
      \brief Decodes ZLib compressed data.
   */
  class ZLibDecoder {
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
  struct Inverse<ZLibDecoder> {
    using type = ZLibEncoder;
  };

  inline std::size_t ZLibDecoder::Decode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    auto zlibDestinationSize = static_cast<uLongf>(destinationSize);
    auto result = uncompress(reinterpret_cast<Bytef*>(destination),
      &zlibDestinationSize, reinterpret_cast<const Bytef*>(source), sourceSize);
    if(result != Z_OK) {
      if(result == Z_BUF_ERROR) {
        BOOST_THROW_EXCEPTION(DecoderException(
          "The buffer was not large enough to hold the uncompressed data."));
      } else if(result == Z_MEM_ERROR) {
        BOOST_THROW_EXCEPTION(DecoderException("Insufficient memory."));
      } else if(result == Z_DATA_ERROR) {
        BOOST_THROW_EXCEPTION(DecoderException(
          "The compressed data was corrupted."));
      } else {
        BOOST_THROW_EXCEPTION(DecoderException("Unknown error."));
      }
    }
    return static_cast<std::size_t>(zlibDestinationSize);
  }

  template<typename Buffer>
  std::size_t ZLibDecoder::Decode(const Buffer& source, void* destination,
      std::size_t destinationSize) {
    return Decode(source.GetData(), source.GetSize(), destination,
      destinationSize);
  }

  template<typename Buffer>
  std::size_t ZLibDecoder::Decode(const void* source, std::size_t sourceSize,
      Out<Buffer> destination) {
    auto originalSize = destination->GetSize();
    auto destinationSize = static_cast<uLongf>(originalSize);
    auto result = uncompress(
      reinterpret_cast<Bytef*>(destination->GetMutableData()), &destinationSize,
      reinterpret_cast<const Bytef*>(source), sourceSize);
    if(result != Z_OK) {
      if(result == Z_BUF_ERROR) {
        BOOST_THROW_EXCEPTION(DecoderException(
          "The buffer was not large enough to hold the uncompressed data."));
      } else if(result == Z_MEM_ERROR) {
        BOOST_THROW_EXCEPTION(DecoderException("Insufficient memory."));
      } else if(result == Z_DATA_ERROR) {
        BOOST_THROW_EXCEPTION(DecoderException(
          "The compressed data was corrupted."));
      } else {
        BOOST_THROW_EXCEPTION(DecoderException("Unknown error."));
      }
    }
    destination->Shrink(destination->GetSize() -
      static_cast<std::size_t>(destinationSize));
    return static_cast<std::size_t>(destinationSize);
  }

  template<typename SourceBuffer, typename DestinationBuffer>
  std::size_t ZLibDecoder::Decode(const SourceBuffer& source,
      Out<DestinationBuffer> destination) {
    return Decode(source.GetData(), source.GetSize(), Store(destination));
  }
}

  template<>
  struct ImplementsConcept<Codecs::ZLibDecoder, Codecs::Decoder> :
    std::true_type {};
}

#endif
