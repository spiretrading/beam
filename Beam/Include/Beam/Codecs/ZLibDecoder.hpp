#ifndef BEAM_ZLIB_DECODER_HPP
#define BEAM_ZLIB_DECODER_HPP
#include <limits>
#include <boost/throw_exception.hpp>
#include <zlib.h>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/IO/Buffer.hpp"

namespace Beam {
  class ZLibEncoder;

  /** Decodes ZLib compressed data. */
  class ZLibDecoder {
    public:
      template<IsConstBuffer S, IsBuffer B>
      std::size_t decode(const S source, Out<B> destination);
  };

  template<>
  struct inverse<ZLibDecoder> {
    using type = ZLibEncoder;
  };

  template<IsConstBuffer S, IsBuffer B>
  std::size_t ZLibDecoder::decode(const S source, Out<B> destination) {
    auto source_size = source.get_size();
    if(source_size == 0) {
      reset(*destination);
      return 0;
    } else if(source_size > std::numeric_limits<uInt>::max()) {
      boost::throw_with_location(
        DecoderException("Source size too large for zlib."));
    }
    const auto MAX_FACTOR = std::size_t(1032);
    reserve(*destination, 10 * source_size);
    if(destination->get_size() < 10 * source_size) {
      boost::throw_with_location(DecoderException("Insufficient space."));
    }
    while(destination->get_size() < MAX_FACTOR * source_size) {
      auto destination_size = destination->get_size();
      auto stream = z_stream();
      stream.zalloc = Z_NULL;
      stream.zfree = Z_NULL;
      stream.opaque = Z_NULL;
      stream.avail_in = static_cast<uInt>(source_size);
      stream.avail_out = static_cast<uInt>(std::min<std::size_t>(
        destination_size, std::numeric_limits<uInt>::max()));
      stream.next_in =
        const_cast<Bytef*>(reinterpret_cast<const Bytef*>(source.get_data()));
      stream.next_out =
        reinterpret_cast<Bytef*>(destination->get_mutable_data());
      auto result = inflateInit(&stream);
      if(result == Z_OK) {
        result = inflate(&stream, Z_FINISH);
        if(result == Z_STREAM_END) {
          result = inflateEnd(&stream);
        }
      }
      if(result == Z_OK) {
        auto produced = static_cast<std::size_t>(stream.total_out);
        destination->shrink(destination->get_size() - produced);
        return produced;
      }
      if(result == Z_BUF_ERROR) {
        auto grow_by = std::max<std::size_t>(destination->get_size(), 1024u);
        auto available_size = destination->grow(grow_by);
        if(available_size < grow_by) {
          boost::throw_with_location(DecoderException("Insufficient space."));
        }
      } else if(result == Z_MEM_ERROR) {
        boost::throw_with_location(DecoderException("Insufficient memory."));
      } else if(result == Z_DATA_ERROR) {
        boost::throw_with_location(
          DecoderException("The compressed data was corrupted."));
      } else {
        boost::throw_with_location(DecoderException("Unknown error."));
      }
    }
    boost::throw_with_location(DecoderException("Unknown error."));
  }
}

#endif
