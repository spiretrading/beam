#ifndef BEAM_SIZE_DECLARATIVE_ENCODER_HPP
#define BEAM_SIZE_DECLARATIVE_ENCODER_HPP
#include <cstdint>
#include <limits>
#include <boost/endian.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/Codecs/EncoderException.hpp"
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/SuffixBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
  template<typename D> requires IsDecoder<dereference_t<D>>
  class SizeDeclarativeDecoder;

  /**
   * Augments an existing encoder by first prepending the size of the buffer.
   * @tparam E The type used to encode the remainder of the buffer.
   */
  template<typename E> requires IsEncoder<dereference_t<E>>
  class SizeDeclarativeEncoder {
    public:

      /** The type used to encode the remainder of the buffer. */
      using Encoder = E;

      /** Constructs a SizeDeclarativeEncoder. */
      SizeDeclarativeEncoder() requires std::default_initializable<Encoder> =
        default;

      /**
       * Constructs a SizeDeclarativeEncoder.
       * @param encoder The underlaying Encoder to use.
       */
      template<Initializes<E> EF>
      explicit SizeDeclarativeEncoder(EF&& encoder);

      template<IsConstBuffer S, IsBuffer B>
      std::size_t encode(const S& source, Out<B> destination);

    private:
      local_ptr_t<E> m_encoder;
  };

  template<typename E>
  struct inverse<SizeDeclarativeEncoder<E>> {
    using type = SizeDeclarativeDecoder<inverse_t<E>>;
  };

  template<typename E> requires IsEncoder<dereference_t<E>>
  template<Initializes<E> EF>
  SizeDeclarativeEncoder<E>::SizeDeclarativeEncoder(EF&& encoder)
    : m_encoder(std::move(encoder)) {}

  template<typename E> requires IsEncoder<dereference_t<E>>
  template<IsConstBuffer S, IsBuffer B>
  std::size_t SizeDeclarativeEncoder<E>::encode(
      const S& source, Out<B> destination) {
    auto portable_size = boost::endian::native_to_big(
      static_cast<std::uint32_t>(source.get_size()));
    auto available_size =
      reserve(*destination, source.get_size() + sizeof(portable_size));
    if (available_size < source.get_size() + sizeof(portable_size)) {
      boost::throw_with_location(EncoderException(
        "The destination was not large enough to hold the encoded data."));
    }
    auto destination_view =
      SuffixBuffer(Ref(*destination), sizeof(portable_size));
    auto encoded_size = m_encoder->encode(source, out(destination_view));
    write(*destination, 0, portable_size);
    return static_cast<std::size_t>(encoded_size + sizeof(portable_size));
  }
}

#endif
