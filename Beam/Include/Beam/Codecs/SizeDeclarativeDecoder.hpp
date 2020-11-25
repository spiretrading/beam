#ifndef BEAM_SIZE_DECLARATIVE_DECODER_HPP
#define BEAM_SIZE_DECLARATIVE_DECODER_HPP
#include <cstdint>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/IO/Buffer.hpp"
#include "Beam/Utilities/Endian.hpp"

namespace Beam {
namespace Codecs {

  /**
   * Augments an existing decoder by first decoding a size from a buffer and
   * then decoding the contents.
   * @param <D> The type used to decode the remainder of the buffer.
   */
  template<typename D>
  class SizeDeclarativeDecoder {
    public:

      /** The type used to decode the remainder of the buffer. */
      using Decoder = D;

      /** Constructs a SizeDeclarativeDecoder. */
      SizeDeclarativeDecoder() = default;

      /**
       * Constructs a SizeDeclarativeDecoder.
       * @param decoder The underlying Decoder to use.
       */
      SizeDeclarativeDecoder(Decoder decoder);

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

    private:
      Decoder m_decoder;
  };

  template<typename D>
  struct Inverse<SizeDeclarativeDecoder<D>> {
    using type = SizeDeclarativeEncoder<GetInverse<D>>;
  };

  template<typename D>
  SizeDeclarativeDecoder<D>::SizeDeclarativeDecoder(Decoder decoder)
    : m_decoder(std::move(decoder)) {}

  template<typename D>
  std::size_t SizeDeclarativeDecoder<D>::Decode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    if(sourceSize < sizeof(std::uint32_t)) {
      BOOST_THROW_EXCEPTION(DecoderException("Source size too small."));
    }
    auto nativeOriginalSize = std::uint32_t();
    std::memcpy(reinterpret_cast<char*>(&nativeOriginalSize), source,
      sizeof(nativeOriginalSize));
    auto originalSize = FromBigEndian(nativeOriginalSize);
    if(destinationSize < originalSize) {
      BOOST_THROW_EXCEPTION(DecoderException("Destination size too small."));
    }
    return m_decoder.Decode(reinterpret_cast<const char*>(source) +
      sizeof(std::uint32_t), sourceSize - sizeof(std::uint32_t), destination,
      destinationSize);
  }

  template<typename D>
  template<typename Buffer>
  std::size_t SizeDeclarativeDecoder<D>::Decode(const Buffer& source,
      void* destination, std::size_t destinationSize) {
    return Decode(source.GetData(), source.GetSize(), destination,
      destinationSize);
  }

  template<typename D>
  template<typename Buffer>
  std::size_t SizeDeclarativeDecoder<D>::Decode(const void* source,
      std::size_t sourceSize, Out<Buffer> destination) {
    if(sourceSize < sizeof(std::uint32_t)) {
      BOOST_THROW_EXCEPTION(DecoderException("Source size too small."));
    }
    auto nativeOriginalSize = std::uint32_t();
    std::memcpy(reinterpret_cast<char*>(&nativeOriginalSize), source,
      sizeof(nativeOriginalSize));
    auto originalSize = FromBigEndian(nativeOriginalSize);
    try {
      destination->Reserve(originalSize);
    } catch(const std::exception&) {
      std::throw_with_nested(DecoderException());
    }
    return m_decoder.Decode(reinterpret_cast<const char*>(source) +
      sizeof(std::uint32_t), sourceSize - sizeof(std::uint32_t),
      Store(destination));
  }

  template<typename D>
  template<typename SourceBuffer, typename DestinationBuffer>
  std::size_t SizeDeclarativeDecoder<D>::Decode(
      const SourceBuffer& source, Out<DestinationBuffer> destination) {
    return Decode(source.GetData(), source.GetSize(), Store(destination));
  }
}

  template<typename D>
  struct ImplementsConcept<Codecs::SizeDeclarativeDecoder<D>, Codecs::Decoder> :
    std::true_type {};
}

#endif
