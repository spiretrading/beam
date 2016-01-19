#ifndef BEAM_SIZEDECLARATIVEDECODER_HPP
#define BEAM_SIZEDECLARATIVEDECODER_HPP
#include <cstdint>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/IO/Buffer.hpp"
#include "Beam/Utilities/Endian.hpp"

namespace Beam {
namespace Codecs {

  /*! \class SizeDeclarativeDecoder
      \brief Decodes a message whose size is stored as a prefix.
   */
  template<typename DecoderType>
  class SizeDeclarativeDecoder {
    public:

      //! The underlaying Decoder.
      using Decoder = DecoderType;

      //! Constructs a SizeDeclarativeDecoder.
      SizeDeclarativeDecoder() = default;

      //! Constructs a SizeDeclarativeDecoder.
      /*!
        \param decoder The underlying Decoder to use.
      */
      SizeDeclarativeDecoder(const Decoder& decoder);

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

  template<typename DecoderType>
  struct Inverse<SizeDeclarativeDecoder<DecoderType>> {
    using type = SizeDeclarativeEncoder<GetInverse<DecoderType>>;
  };

  template<typename DecoderType>
  SizeDeclarativeDecoder<DecoderType>::SizeDeclarativeDecoder(
      const Decoder& decoder)
      : m_decoder(decoder) {}

  template<typename DecoderType>
  std::size_t SizeDeclarativeDecoder<DecoderType>::Decode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    if(sourceSize < sizeof(std::uint32_t)) {
      BOOST_THROW_EXCEPTION(DecoderException("Source size too small."));
    }
    std::uint32_t nativeOriginalSize;
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

  template<typename DecoderType>
  template<typename Buffer>
  std::size_t SizeDeclarativeDecoder<DecoderType>::Decode(const Buffer& source,
      void* destination, std::size_t destinationSize) {
    return Decode(source.GetData(), source.GetSize(), destination,
      destinationSize);
  }

  template<typename DecoderType>
  template<typename Buffer>
  std::size_t SizeDeclarativeDecoder<DecoderType>::Decode(const void* source,
      std::size_t sourceSize, Out<Buffer> destination) {
    if(sourceSize < sizeof(std::uint32_t)) {
      BOOST_THROW_EXCEPTION(DecoderException("Source size too small."));
    }
    std::uint32_t nativeOriginalSize;
    std::memcpy(reinterpret_cast<char*>(&nativeOriginalSize), source,
      sizeof(nativeOriginalSize));
    auto originalSize = FromBigEndian(nativeOriginalSize);
    destination->Reserve(originalSize);
    return m_decoder.Decode(reinterpret_cast<const char*>(source) +
      sizeof(std::uint32_t), sourceSize - sizeof(std::uint32_t),
      Store(destination));
  }

  template<typename DecoderType>
  template<typename SourceBuffer, typename DestinationBuffer>
  std::size_t SizeDeclarativeDecoder<DecoderType>::Decode(
      const SourceBuffer& source, Out<DestinationBuffer> destination) {
    return Decode(source.GetData(), source.GetSize(), Store(destination));
  }
}

  template<typename DecoderType>
  struct ImplementsConcept<Codecs::SizeDeclarativeDecoder<DecoderType>,
    Codecs::Decoder> : std::true_type {};
}

#endif
