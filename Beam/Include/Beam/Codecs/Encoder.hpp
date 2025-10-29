#ifndef BEAM_ENCODER_HPP
#define BEAM_ENCODER_HPP
#include <concepts>
#include <cstdint>
#include <memory>
#include "Beam/IO/BufferRef.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Determines whether a type meets the Encoder concept. */
  template<typename T>
  concept IsEncoder = requires(T& encoder) {
    { encoder.encode(std::declval<BufferCRef>(),
        out(std::declval<BufferRef&>())) } -> std::convertible_to<std::size_t>;
  };

  /** Interface for encoding data. */
  class Encoder {
    public:

      /**
       * Constructs an Encoder of a specified type using emplacement.
       * @tparam T The type of encoder to emplace.
       * @param args The arguments to pass to the emplaced encoder.
       */
      template<IsEncoder T, typename... Args>
      explicit Encoder(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an Encoder by referencing an existing encoder.
       * @param encoder The encoder to reference.
       */
      template<DisableCopy<Encoder> T> requires IsEncoder<dereference_t<T>>
      Encoder(T&& encoder);

      Encoder(const Encoder&) = default;

      /**
       * Encodes data to a specified destination.
       * @param source The source data to encode.
       * @param destination The destination of the encoded <i>source</i>.
       * @return The number of bytes encoded into the <i>destination</i>.
       */
      template<IsConstBuffer S, IsBuffer B>
      std::size_t encode(const S& source, Out<B> destination);

    private:
      struct VirtualEncoder {
        virtual ~VirtualEncoder() = default;

        virtual std::size_t encode(
          BufferCRef source, BufferRef destination) = 0;
      };
      template<typename E>
      struct WrappedEncoder final : VirtualEncoder {
        using Encoder = E;
        local_ptr_t<Encoder> m_encoder;

        template<typename... Args>
        WrappedEncoder(Args&&... args);

        std::size_t encode(BufferCRef source, BufferRef destination) override;
      };
      VirtualPtr<VirtualEncoder> m_encoder;
  };

  /** Specifies whether in-place encoding is supported. */
  template<typename T>
  struct in_place_support : std::false_type {};

  template<typename T>
  constexpr auto in_place_support_v = in_place_support<T>::value;

  template<IsEncoder T, typename... Args>
  Encoder::Encoder(std::in_place_type_t<T>, Args&&... args)
    : m_encoder(
        make_virtual_ptr<WrappedEncoder<T>>(std::forward<Args>(args)...)) {}

  template<DisableCopy<Encoder> T> requires IsEncoder<dereference_t<T>>
  Encoder::Encoder(T&& encoder)
    : m_encoder(make_virtual_ptr<WrappedEncoder<std::remove_cvref_t<T>>>(
        std::forward<T>(encoder))) {}

  template<IsConstBuffer S, IsBuffer B>
  std::size_t Encoder::encode(const S& source, Out<B> destination) {
    return m_encoder->encode(source, *destination);
  }

  template<typename E>
  template<typename... Args>
  Encoder::WrappedEncoder<E>::WrappedEncoder(Args&&... args)
    : m_encoder(std::forward<Args>(args)...) {}

  template<typename E>
  std::size_t Encoder::WrappedEncoder<E>::encode(
      BufferCRef source, BufferRef destination) {
    return m_encoder->encode(source, out(destination));
  }
}

#endif
