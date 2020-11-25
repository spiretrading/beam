#ifndef BEAM_SIZE_DECLARATIVE_ENCODER_HPP
#define BEAM_SIZE_DECLARATIVE_ENCODER_HPP
#include <cstdint>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Codecs/EncoderException.hpp"
#include "Beam/IO/BufferSlice.hpp"
#include "Beam/Utilities/Endian.hpp"

namespace Beam {
namespace Codecs {

  /**
   * Augments an existing encoder by first prepending the size of the buffer.
   * @param <E> The type used to encode the remainder of the buffer.
   */
  template<typename E>
  class SizeDeclarativeEncoder {
    public:

      /** The type used to encode the remainder of the buffer. */
      using Encoder = E;

      /** Constructs a SizeDeclarativeEncoder. */
      SizeDeclarativeEncoder() = default;

      /**
       * Constructs a SizeDeclarativeEncoder.
       * @param encoder The underlaying Encoder to use.
       */
      SizeDeclarativeEncoder(Encoder encoder);

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

  template<typename E>
  struct Inverse<SizeDeclarativeEncoder<E>> {
    using type = SizeDeclarativeDecoder<GetInverse<E>>;
  };

  template<typename E>
  SizeDeclarativeEncoder<E>::SizeDeclarativeEncoder(Encoder encoder)
    : m_encoder(std::move(encoder)) {}

  template<typename E>
  std::size_t SizeDeclarativeEncoder<E>::Encode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    if(destinationSize < sizeof(std::uint32_t)) {
      BOOST_THROW_EXCEPTION(EncoderException("Destination size is too small."));
    }
    auto portableSourceSize = ToBigEndian<std::uint32_t>(sourceSize);
    std::memcpy(destination, reinterpret_cast<const char*>(&portableSourceSize),
      sizeof(portableSourceSize));
    auto messageDestination = reinterpret_cast<char*>(destination) +
      sizeof(std::uint32_t);
    return m_encoder.Encode(source, sourceSize, messageDestination,
      destinationSize - sizeof(std::uint32_t)) + sizeof(std::uint32_t);
  }

  template<typename E>
  template<typename Buffer>
  std::size_t SizeDeclarativeEncoder<E>::Encode(const Buffer& source,
      void* destination, std::size_t destinationSize) {
    return Encode(source.GetData(), source.GetSize(), destination,
      destinationSize);
  }

  template<typename E>
  template<typename Buffer>
  std::size_t SizeDeclarativeEncoder<E>::Encode(const void* source,
      std::size_t sourceSize, Out<Buffer> destination) {
    try {
      destination->Append(ToBigEndian<std::uint32_t>(sourceSize));
    } catch(const std::exception&) {
      std::throw_with_nested(EncoderException());
    }
    auto destinationView = IO::BufferSlice(Ref(*destination),
      sizeof(std::uint32_t));
    return m_encoder.Encode(source, sourceSize, Store(destinationView)) +
      sizeof(std::uint32_t);
  }

  template<typename E>
  template<typename SourceBuffer, typename DestinationBuffer>
  std::size_t SizeDeclarativeEncoder<E>::Encode(
      const SourceBuffer& source, Out<DestinationBuffer> destination) {
    return Encode(source.GetData(), source.GetSize(), Store(destination));
  }
}

  template<typename E>
  struct ImplementsConcept<Codecs::SizeDeclarativeEncoder<E>, Codecs::Encoder> :
    std::true_type {};
}

#endif
