#ifndef BEAM_DECODER_HPP
#define BEAM_DECODER_HPP
#include <concepts>
#include <cstdint>
#include <memory>
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/IO/BufferRef.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Determines whether a type meets the Decoder concept. */
  template<typename T>
  concept IsDecoder = requires(T& decoder) {
    { decoder.decode(std::declval<BufferCRef>(),
        out(std::declval<BufferRef&>())) } -> std::convertible_to<std::size_t>;
  };

  /** Interface for decoding data. */
  class Decoder {
    public:

      /**
       * Constructs a Decoder of a specified type using emplacement.
       * @tparam T The type of decoder to emplace.
       * @param args The arguments to pass to the emplaced decoder.
       */
      template<IsDecoder T, typename... Args>
      explicit Decoder(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a Decoder by referencing an existing decoder.
       * @param decoder The decoder to reference.
       */
      template<DisableCopy<Decoder> T> requires IsDecoder<dereference_t<T>>
      Decoder(T&& decoder);

      Decoder(const Decoder&) = default;

      /**
       * Decodes data to a specified destination.
       * @param source The source data to decode.
       * @param destination The destination of the decoded <i>source</i>.
       */
      template<IsConstBuffer S, IsBuffer B>
      std::size_t decode(const S source, Out<B> destination);

    private:
      struct VirtualDecoder {
        virtual ~VirtualDecoder() = default;

        virtual std::size_t decode(
          BufferCRef source, BufferRef destination) = 0;
      };
      template<typename D>
      struct WrappedDecoder final : VirtualDecoder {
        using Decoder = D;
        local_ptr_t<Decoder> m_decoder;

        template<typename... Args>
        WrappedDecoder(Args&&... args);

        std::size_t decode(BufferCRef source, BufferRef destination) override;
      };
      VirtualPtr<VirtualDecoder> m_decoder;
  };

  template<IsDecoder T, typename... Args>
  Decoder::Decoder(std::in_place_type_t<T>, Args&&... args)
    : m_decoder(
        make_virtual_ptr<WrappedDecoder<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<Decoder> T> requires IsDecoder<dereference_t<T>>
  Decoder::Decoder(T&& decoder)
    : m_decoder(make_virtual_ptr<WrappedDecoder<std::remove_cvref_t<T>>>(
        std::forward<T>(decoder))) {}

  template<IsConstBuffer S, IsBuffer B>
  std::size_t Decoder::decode(const S source, Out<B> destination) {
    return m_decoder->decode(source, *destination);
  }

  template<typename D>
  template<typename... Args>
  Decoder::WrappedDecoder<D>::WrappedDecoder(Args&&... args)
    : m_decoder(std::forward<Args>(args)...) {}

  template<typename D>
  std::size_t Decoder::WrappedDecoder<D>::decode(
      BufferCRef source, BufferRef destination) {
    return m_decoder->decode(source, out(destination));
  }
}

#endif
