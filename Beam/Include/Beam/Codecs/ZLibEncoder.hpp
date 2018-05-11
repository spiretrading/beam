#ifndef BEAM_ZLIBENCODER_HPP
#define BEAM_ZLIBENCODER_HPP
#include <cassert>
#include <boost/throw_exception.hpp>
#include <zlib.h>
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Codecs/EncoderException.hpp"
#include "Beam/IO/Buffer.hpp"

namespace Beam {
namespace Codecs {

  /*! \class ZLibEncoder
      \brief Encodes using ZLib compression.
   */
  class ZLibEncoder {
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
  struct Inverse<ZLibEncoder> {
    using type = ZLibDecoder;
  };

  inline std::size_t ZLibEncoder::Encode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = static_cast<uInt>(sourceSize);
    stream.next_in = static_cast<Bytef*>(const_cast<void*>(source));
    stream.avail_out = static_cast<uInt>(destinationSize);
    stream.next_out = static_cast<Bytef*>(const_cast<void*>(destination));
    auto result = deflateInit(&stream, Z_BEST_COMPRESSION);
    if(result == Z_OK) {
      result = deflate(&stream, Z_FINISH);
      if(result == Z_STREAM_END) {
        result = deflateEnd(&stream);
      }
    }
    if(result != Z_OK) {
      if(result == Z_BUF_ERROR) {
        BOOST_THROW_EXCEPTION(EncoderException(
          "The buffer was not large enough to hold the compressed data."));
      } else if(result == Z_MEM_ERROR) {
        BOOST_THROW_EXCEPTION(EncoderException("Insufficient memory."));
      } else {
        BOOST_THROW_EXCEPTION(EncoderException("Unknown error."));
      }
    }
    return static_cast<std::size_t>(stream.total_out);
  }

  template<typename Buffer>
  std::size_t ZLibEncoder::Encode(const Buffer& source, void* destination,
      std::size_t destinationSize) {
    return Encode(source.GetData(), source.GetSize(), destination,
      destinationSize);
  }

  template<typename Buffer>
  std::size_t ZLibEncoder::Encode(const void* source, std::size_t sourceSize,
      Out<Buffer> destination) {
    auto sizeEstimate = static_cast<std::size_t>(
      deflateBound(nullptr, sourceSize));
    destination->Reserve(sizeEstimate);
    auto size = Encode(source, sourceSize, destination->GetMutableData(),
      destination->GetSize());
    destination->Shrink(destination->GetSize() - size);
    return size;
  }

  template<typename SourceBuffer, typename DestinationBuffer>
  std::size_t ZLibEncoder::Encode(const SourceBuffer& source,
      Out<DestinationBuffer> destination) {
    return Encode(source.GetData(), source.GetSize(), Store(destination));
  }
}

  template<>
  struct ImplementsConcept<Codecs::ZLibEncoder, Codecs::Encoder> :
    std::true_type {};
}

#endif
