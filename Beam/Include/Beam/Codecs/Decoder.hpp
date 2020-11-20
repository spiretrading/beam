#ifndef BEAM_DECODER_HPP
#define BEAM_DECODER_HPP
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Utilities/Concept.hpp"

namespace Beam::Codecs {

  /** Interface for decoding data. */
  struct Decoder : Concept<Decoder> {

    /**
     * Decodes data to a specified destination.
     * @param source The source data to decode.
     * @param sourceSize The size of the <i>source</i>.
     * @param destination The destination of the decoded <i>source</i>.
     * @param destinationSize The size of the <i>destination</i>.
     */
    std::size_t Decode(const void* source, std::size_t sourceSize,
      void* destination, std::size_t destinationSize);

    /**
     * Decodes data to a specified destination.
     * @tparam <Buffer> The type of Buffer to decode.
     * @param source The source data to decode.
     * @param destination The destination of the decoded <i>source</i>.
     * @param destinationSize The size of the <i>destination</i>.
     */
    template<typename Buffer>
    std::size_t Decode(const Buffer& source, void* destination,
      std::size_t destinationSize);

    /**
     * Decodes data to a specified destination.
     * @tparam <Buffer> The type of Buffer to store the decoding.
     * @param source The source data to decode.
     * @param sourceSize The size of the source data.
     * @param destination The destination of the decoded <i>source</i>.
     */
    template<typename Buffer>
    std::size_t Decode(const void* source, std::size_t sourceSize,
      Out<Buffer> destination);

    /**
     * Decodes data to a specified destination.
     * @tparam <SourceBuffer> The type of Buffer to decode.
     * @tparam <DestinationBuffer> The type of Buffer to store the decoding.
     * \param source The source data to decode.
     * \param destination The destination of the decoded <i>source</i>.
     */
    template<typename SourceBuffer, typename DestinationBuffer>
    std::size_t Decode(const SourceBuffer& source,
      Out<DestinationBuffer> destination);
  };
}

#endif
