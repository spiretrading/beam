#ifndef BEAM_ZLIB_ENCODER_HPP
#define BEAM_ZLIB_ENCODER_HPP
#include <cassert>
#include <limits>
#include <boost/throw_exception.hpp>
#include <zlib.h>
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Codecs/EncoderException.hpp"

namespace Beam {
  class ZLibDecoder;

  /** Encodes using ZLib compression. */
  class ZLibEncoder {
    public:
      template<IsConstBuffer S, IsBuffer B>
      std::size_t encode(const S& source, Out<B> destination);
  };

  template<>
  struct inverse<ZLibEncoder> {
    using type = ZLibDecoder;
  };

  template<IsConstBuffer S, IsBuffer B>
  std::size_t ZLibEncoder::encode(const S& source, Out<B> destination) {
    auto input_size = source.get_size();
    if(input_size > std::numeric_limits<uLong>::max()) {
      boost::throw_with_location(EncoderException("Source size too large."));
    }
    auto required_size = compressBound(static_cast<uLong>(input_size));
    auto available_size = reserve(*destination, required_size);
    if(available_size < required_size) {
      boost::throw_with_location(EncoderException("Insufficient space."));
    }
    while(true) {
      auto destination_size = std::min<uInt>(
        destination->get_size(), std::numeric_limits<uInt>::max());
      auto stream = z_stream();
      stream.zalloc = Z_NULL;
      stream.zfree = Z_NULL;
      stream.opaque = Z_NULL;
      stream.avail_in = static_cast<uInt>(input_size);
      stream.next_in =
        reinterpret_cast<Bytef*>(const_cast<char*>(source.get_data()));
      stream.avail_out = static_cast<uInt>(destination_size);
      stream.next_out = reinterpret_cast<Bytef*>(
        const_cast<char*>(destination->get_mutable_data()));
      auto result = deflateInit(&stream, Z_BEST_COMPRESSION);
      if(result == Z_OK) {
        result = deflate(&stream, Z_FINISH);
        if(result == Z_STREAM_END) {
          result = deflateEnd(&stream);
        }
      }
      if(result == Z_OK) {
        destination->shrink(destination->get_size() - stream.total_out);
        return stream.total_out;
      }
      if(result == Z_BUF_ERROR) {
        auto grow_by = std::max<std::size_t>(destination->get_size(), 1024);
        auto available_size = destination->grow(grow_by);
        if(available_size < grow_by) {
          boost::throw_with_location(DecoderException("Insufficient space."));
        }
      } else if(result == Z_MEM_ERROR) {
        boost::throw_with_location(EncoderException("Insufficient memory."));
      } else {
        boost::throw_with_location(EncoderException("Unknown error."));
      }
    }
  }
}

#endif
