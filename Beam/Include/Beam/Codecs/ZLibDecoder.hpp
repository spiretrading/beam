#ifndef BEAM_ZLIB_DECODER_HPP
#define BEAM_ZLIB_DECODER_HPP
#include <limits>
#include <sstream>
#include <boost/scope/scope_exit.hpp>
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
    auto maximum_size = [&] {
      constexpr auto MAX_FACTOR = std::size_t(1032);
      if(source_size > std::numeric_limits<std::size_t>::max() / MAX_FACTOR) {
        return std::numeric_limits<std::size_t>::max();
      }
      return MAX_FACTOR * source_size;
    }();
    reserve(*destination, std::min(source_size, maximum_size / 10) * 10);
    auto stream = z_stream();
    stream.avail_in = static_cast<uInt>(source_size);
    stream.next_in =
      const_cast<Bytef*>(reinterpret_cast<const Bytef*>(source.get_data()));
    auto produced = std::size_t(0);
    auto result = inflateInit(&stream);
    auto fail = [&] (const char* reason) {
      auto message = std::ostringstream();
      message << reason << " zlib_result=" << result;
      if(stream.msg) {
        message << " zlib_message=" << stream.msg;
      }
      message << " compressed_size=" << source_size << " consumed=" <<
        source_size - stream.avail_in << " produced=" << produced <<
        " destination_size=" << destination->get_size();
      boost::throw_with_location(DecoderException(message.str()));
    };
    if(result != Z_OK) {
      fail("Unable to initialize zlib decoder.");
    }
    auto cleanup = boost::scope::scope_exit([&] {
      inflateEnd(&stream);
    });
    while(true) {
      auto destination_size = std::min(destination->get_size(), maximum_size);
      if(produced == destination_size) {
        if(destination_size == maximum_size) {
          fail("Zlib decompression limit exceeded.");
        }
        auto growth = std::min(std::max(destination_size, std::size_t(1024)),
          maximum_size - destination_size);
        if(destination->grow(growth) == 0) {
          fail("Insufficient space for decompressed data.");
        }
        destination_size = std::min(destination->get_size(), maximum_size);
      }
      auto output_size = static_cast<uInt>(std::min<std::size_t>(
        destination_size - produced, std::numeric_limits<uInt>::max()));
      stream.avail_out = output_size;
      stream.next_out = reinterpret_cast<Bytef*>(
        destination->get_mutable_data() + produced);
      result = inflate(&stream, Z_FINISH);
      produced += output_size - stream.avail_out;
      if(result == Z_STREAM_END) {
        if(stream.avail_in != 0) {
          fail("The compressed data contains trailing bytes.");
        }
        destination->shrink(destination->get_size() - produced);
        return produced;
      }
      if(result == Z_MEM_ERROR) {
        fail("Insufficient memory for zlib decoder.");
      } else if(result == Z_DATA_ERROR) {
        fail("The compressed data was corrupted.");
      } else if(result != Z_OK && result != Z_BUF_ERROR) {
        fail("Unable to decompress zlib data.");
      }
      if(stream.avail_out != 0) {
        if(stream.avail_in == 0) {
          fail("The compressed data was truncated.");
        }
        fail("Zlib decoder made no progress.");
      }
    }
  }
}

#endif
