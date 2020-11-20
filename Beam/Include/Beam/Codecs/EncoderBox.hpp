#ifndef BEAM_ENCODER_BOX_HPP
#define BEAM_ENCODER_BOX_HPP
#include <utility>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace Codecs {

  /** Provides a generic interface to an arbitrary Encoder */
  class EncoderBox {
    public:

      /**
       * Constructs an EncoderBox of a specified type using emplacement.
       * @param <T> The type of encoder to emplace.
       * @param args The arguments to pass to the emplaced encoder.
       */
      template<typename T, typename... Args>
      EncoderBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs an EncoderBox by copying an existing Encoder.
       * @param encoder The encoder to copy.
       */
      template<typename Encoder>
      EncoderBox(Encoder encoder);

      std::size_t Encode(const void* source, std::size_t sourceSize,
        void* destination, std::size_t destinationSize);

      std::size_t Encode(const IO::SharedBuffer& source, void* destination,
        std::size_t destinationSize);

      std::size_t Encode(const void* source, std::size_t sourceSize,
        Out<IO::SharedBuffer> destination);

      std::size_t Encode(const IO::SharedBuffer& source,
        Out<IO::SharedBuffer> destination);

    private:
      struct VirtualEncoder {
        virtual ~VirtualEncoder() = default;
        virtual std::size_t Encode(const void* source, std::size_t sourceSize,
          void* destination, std::size_t destinationSize) = 0;
        virtual std::size_t Encode(const IO::SharedBuffer& source,
          void* destination, std::size_t destinationSize) = 0;
        virtual std::size_t Encode(const void* source, std::size_t sourceSize,
          Out<IO::SharedBuffer> destination) = 0;
        virtual std::size_t Encode(const IO::SharedBuffer& source,
          Out<IO::SharedBuffer> destination) = 0;
      };
      template<typename E>
      struct WrappedEncoder final : VirtualEncoder {
        using Encoder = E;
        GetOptionalLocalPtr<Encoder> m_encoder;

        template<typename... Args>
        WrappedEncoder(Args&&... args);
        std::size_t Encode(const void* source, std::size_t sourceSize,
          void* destination, std::size_t destinationSize) override;
        std::size_t Encode(const IO::SharedBuffer& source, void* destination,
          std::size_t destinationSize) override;
        std::size_t Encode(const void* source, std::size_t sourceSize,
          Out<IO::SharedBuffer> destination) override;
        std::size_t Encode(const IO::SharedBuffer& source,
          Out<IO::SharedBuffer> destination) override;
      };
      std::unique_ptr<VirtualEncoder> m_encoder;

      EncoderBox(const EncoderBox&) = delete;
      EncoderBox& operator =(const EncoderBox&) = delete;
  };

  template<typename T, typename... Args>
  EncoderBox::EncoderBox(std::in_place_type_t<T>, Args&&... args)
    : m_encoder(std::make_unique<WrappedEncoder<T>>(
        std::forward<Args>(args)...)) {}

  template<typename Encoder>
  EncoderBox::EncoderBox(Encoder encoder)
    : EncoderBox(std::in_place_type<Encoder>, std::move(encoder)) {}

  inline std::size_t EncoderBox::Encode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    return m_encoder->Encode(source, sourceSize, destination, destinationSize);
  }

  inline std::size_t EncoderBox::Encode(const IO::SharedBuffer& source,
      void* destination, std::size_t destinationSize) {
    return m_encoder->Encode(source, destination, destinationSize);
  }

  inline std::size_t EncoderBox::Encode(const void* source,
      std::size_t sourceSize, Out<IO::SharedBuffer> destination) {
    return m_encoder->Encode(source, sourceSize, Store(destination));
  }

  inline std::size_t EncoderBox::Encode(const IO::SharedBuffer& source,
      Out<IO::SharedBuffer> destination) {
    return m_encoder->Encode(source, Store(destination));
  }

  template<typename E>
  template<typename... Args>
  EncoderBox::WrappedEncoder<E>::WrappedEncoder(Args&&... args)
    : m_encoder(std::forward<Args>(args)...) {}

  template<typename E>
  std::size_t EncoderBox::WrappedEncoder<E>::Encode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    return m_encoder->Encode(source, sourceSize, destination, destinationSize);
  }

  template<typename E>
  std::size_t EncoderBox::WrappedEncoder<E>::Encode(
      const IO::SharedBuffer& source, void* destination,
      std::size_t destinationSize) {
    return m_encoder->Encode(source, destination, destinationSize);
  }

  template<typename E>
  std::size_t EncoderBox::WrappedEncoder<E>::Encode(const void* source,
      std::size_t sourceSize, Out<IO::SharedBuffer> destination) {
    return m_encoder->Encode(source, sourceSize, Store(destination));
  }

  template<typename E>
  std::size_t EncoderBox::WrappedEncoder<E>::Encode(
      const IO::SharedBuffer& source, Out<IO::SharedBuffer> destination) {
    return m_encoder->Encode(source, Store(destination));
  }
}

  template<>
  struct ImplementsConcept<Codecs::EncoderBox, Codecs::Encoder> :
    std::true_type {};
}

#endif
