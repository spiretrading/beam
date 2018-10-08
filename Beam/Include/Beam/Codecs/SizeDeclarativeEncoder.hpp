#ifndef BEAM_SIZE_DECLARATIVE_ENCODER_HPP
#define BEAM_SIZE_DECLARATIVE_ENCODER_HPP
#include <cstdint>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Codecs/EncoderException.hpp"
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/BufferView.hpp"
#include "Beam/Utilities/Endian.hpp"

namespace Beam {
namespace Codecs {

  /** Encodes a message whose size is stored as a prefix. */
  template<typename EncoderType>
  class SizeDeclarativeEncoder {
    public:

      //! The underlaying Encoder.
      using Encoder = EncoderType;

      //! Constructs a SizeDeclarativeEncoder.
      SizeDeclarativeEncoder() = default;

      //! Constructs a SizeDeclarativeEncoder.
      /*!
        \param encoder The underlaying Encoder to use.
      */
      SizeDeclarativeEncoder(const Encoder& encoder);

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

    private:
      Encoder m_encoder;
  };

  template<typename EncoderType>
  struct Inverse<SizeDeclarativeEncoder<EncoderType>> {
    using type = SizeDeclarativeDecoder<GetInverse<EncoderType>>;
  };

  template<typename EncoderType>
  SizeDeclarativeEncoder<EncoderType>::SizeDeclarativeEncoder(
      const Encoder& encoder)
      : m_encoder(encoder) {}

  template<typename EncoderType>
  std::size_t SizeDeclarativeEncoder<EncoderType>::Encode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    if(destinationSize < sizeof(std::uint32_t)) {
      BOOST_THROW_EXCEPTION(EncoderException("Destination size is too small."));
    }
    auto portableSourceSize = ToBigEndian<std::uint32_t>(sourceSize);
    std::memcpy(destination, reinterpret_cast<const char*>(&portableSourceSize),
      sizeof(portableSourceSize));
    auto messageDestination = reinterpret_cast<char*>(destination) +
      sizeof(std::uint32_t);
    auto encodedSize = m_encoder->Encode(source, sourceSize, messageDestination,
      destinationSize - sizeof(std::uint32_t)) + sizeof(std::uint32_t);
    return encodedSize;
  }

  template<typename EncoderType>
  template<typename Buffer>
  std::size_t SizeDeclarativeEncoder<EncoderType>::Encode(const Buffer& source,
      void* destination, std::size_t destinationSize) {
    return Encode(source.GetData(), source.GetSize(), destination,
      destinationSize);
  }

  template<typename EncoderType>
  template<typename Buffer>
  std::size_t SizeDeclarativeEncoder<EncoderType>::Encode(const void* source,
      std::size_t sourceSize, Out<Buffer> destination) {
    destination->Append(ToBigEndian<std::uint32_t>(sourceSize));
    auto destinationView = IO::BufferView<Buffer>(Ref(*destination),
      sizeof(std::uint32_t));
    auto size = m_encoder.Encode(source, sourceSize, Store(destinationView)) +
      sizeof(std::uint32_t);
    return size;
  }

  template<typename EncoderType>
  template<typename SourceBuffer, typename DestinationBuffer>
  std::size_t SizeDeclarativeEncoder<EncoderType>::Encode(
      const SourceBuffer& source, Out<DestinationBuffer> destination) {
    return Encode(source.GetData(), source.GetSize(), Store(destination));
  }
}

  template<typename EncoderType>
  struct ImplementsConcept<Codecs::SizeDeclarativeEncoder<EncoderType>,
    Codecs::Encoder> : std::true_type {};
}

#endif
