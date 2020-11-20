#ifndef BEAM_ENCODER_HPP
#define BEAM_ENCODER_HPP
#include <type_traits>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Utilities/Concept.hpp"

namespace Beam::Codecs {

  /** Interface for encoding data. */
  struct Encoder : Concept<Encoder> {

    /**
     * Encodes data to a specified destination.
     * @param source The source data to encode.
     * @param sourceSize The size of the <i>source</i>.
     * @param destination The destination of the encoded <i>source</i>.
     * @param destinationSize The size of the <i>destination</i>.
     */
    std::size_t Encode(const void* source, std::size_t sourceSize,
      void* destination, std::size_t destinationSize);

    /**
     * Encodes data to a specified destination.
     * @tparam <Buffer> The type of Buffer to encode.
     * @param source The source data to encode.
     * @param destination The destination of the encoded <i>source</i>.
     * @param destinationSize The size of the <i>destination</i>.
     */
    template<typename Buffer>
    std::size_t Encode(const Buffer& source, void* destination,
      std::size_t destinationSize);

    /**
     * Encodes data to a specified destination.
     * @tparam <Buffer> The type of Buffer to store the encoding.
     * @param source The source data to encode.
     * @param sourceSize The size of the source data.
     * @param destination The destination of the encoded <i>source</i>.
     */
    template<typename Buffer>
    std::size_t Encode(const void* source, std::size_t sourceSize,
      Out<Buffer> destination);

    /**
     * Encodes data to a specified destination.
     * @tparam <SourceBuffer> The type of Buffer to decode.
     * @tparam <DestinationBuffer> The type of Buffer to store the decoding.
     * @param source The source data to encode.
     * @param destination The destination of the encoded <i>source</i>.
     */
    template<typename SourceBuffer, typename DestinationBuffer>
    std::size_t Encode(const SourceBuffer& source,
      Out<DestinationBuffer> destination);
  };

  /**
   * Returns the inverse of a specified codec.
   * @param <C> The codec to get the inverse of.
   */
  template<typename C>
  struct Inverse {};

  template<typename C>
  using GetInverse = typename Inverse<C>::type;

  /** Specifies whether in-place encoding is supported. */
  template<typename T>
  struct InPlaceSupport : std::false_type {};
}

#endif
