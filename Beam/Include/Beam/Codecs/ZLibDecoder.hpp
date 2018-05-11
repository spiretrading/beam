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
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = static_cast<uInt>(sourceSize);
    stream.next_in = static_cast<Bytef*>(const_cast<void*>(source));
    stream.avail_out = static_cast<uInt>(destinationSize);
    stream.next_out = static_cast<Bytef*>(destination);
    auto result = inflateInit(&stream);
    if(result == Z_OK) {
      result = inflate(&stream, Z_FINISH);
      if(result == Z_STREAM_END) {
        result = inflateEnd(&stream);
      }
    }
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
    return static_cast<std::size_t>(stream.total_out);
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
    const auto MAX_FACTOR = 1032;
    destination->Reserve(10 * sourceSize);
    while(destination->GetSize() < MAX_FACTOR * sourceSize) {
      auto destinationSize = destination->GetSize();
      z_stream stream;
      stream.zalloc = Z_NULL;
      stream.zfree = Z_NULL;
      stream.opaque = Z_NULL;
      stream.avail_in = static_cast<uInt>(sourceSize);
      stream.next_in = static_cast<Bytef*>(const_cast<void*>(source));
      stream.avail_out = static_cast<uInt>(destinationSize);
      stream.next_out = reinterpret_cast<Bytef*>(destination->GetMutableData());
      auto result = inflateInit(&stream);
      if(result == Z_OK) {
        result = inflate(&stream, Z_FINISH);
        if(result == Z_STREAM_END) {
          result = inflateEnd(&stream);
        }
      }
      if(result == Z_OK) {
        auto size = static_cast<std::size_t>(stream.total_out);
        destination->Shrink(destination->GetSize() - size);
        return size;
      }
      if(result == Z_BUF_ERROR) {
        destination->Reserve(2 * destination->GetSize());
      } else if(result == Z_MEM_ERROR) {
        BOOST_THROW_EXCEPTION(DecoderException("Insufficient memory."));
      } else if(result == Z_DATA_ERROR) {
        BOOST_THROW_EXCEPTION(DecoderException(
          "The compressed data was corrupted."));
      } else {
        BOOST_THROW_EXCEPTION(DecoderException("Unknown error."));
      }
    }
    BOOST_THROW_EXCEPTION(DecoderException("Unknown error."));
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
