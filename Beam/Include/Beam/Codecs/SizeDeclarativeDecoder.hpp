#ifndef BEAM_SIZE_DECLARATIVE_DECODER_HPP
#define BEAM_SIZE_DECLARATIVE_DECODER_HPP
#include <cstdint>
#include <boost/endian.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/Codecs/DecoderException.hpp"
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/SuffixBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
  template<typename E> requires IsEncoder<dereference_t<E>>
  class SizeDeclarativeEncoder;

  /**
   * Augments an existing decoder by first decoding a size from a buffer and
   * then decoding the contents.
   * @tparam D The type used to decode the remainder of the buffer.
   */
  template<typename D> requires IsDecoder<dereference_t<D>>
  class SizeDeclarativeDecoder {
    public:

      /** The type used to decode the remainder of the buffer. */
      using Decoder = D;

      /** Constructs a SizeDeclarativeDecoder. */
      SizeDeclarativeDecoder() requires std::default_initializable<Decoder> =
        default;

      /**
       * Constructs a SizeDeclarativeDecoder.
       * @param decoder The underlying Decoder to use.
       */
      template<Initializes<D> DF>
      explicit SizeDeclarativeDecoder(DF&& decoder);

      template<IsConstBuffer S, IsBuffer B>
      std::size_t decode(const S source, Out<B> destination);

    private:
      local_ptr_t<D> m_decoder;
  };

  template<typename D>
  struct inverse<SizeDeclarativeDecoder<D>> {
    using type = SizeDeclarativeEncoder<inverse_t<D>>;
  };

  template<typename D> requires IsDecoder<dereference_t<D>>
  template<Initializes<D> DF>
  SizeDeclarativeDecoder<D>::SizeDeclarativeDecoder(DF&& decoder)
    : m_decoder(std::forward<DF>(decoder)) {}

  template<typename D> requires IsDecoder<dereference_t<D>>
  template<IsConstBuffer S, IsBuffer B>
  std::size_t SizeDeclarativeDecoder<D>::decode(
      const S source, Out<B> destination) {
    if(source.get_size() < sizeof(std::uint32_t)) {
      boost::throw_with_location(DecoderException("Source size too small."));
    }
    auto native_length = std::uint32_t();
    std::memcpy(reinterpret_cast<char*>(&native_length), source.get_data(),
      sizeof(native_length));
    auto original_length =
      static_cast<std::size_t>(boost::endian::big_to_native(native_length));
    auto available_size = reserve(*destination, original_length);
    if(available_size < original_length) {
      boost::throw_with_location(
        DecoderException("Destination size too small."));
    }
    return m_decoder->decode(
      suffix(Ref(source), sizeof(native_length)), out(destination));
  }
}

#endif
