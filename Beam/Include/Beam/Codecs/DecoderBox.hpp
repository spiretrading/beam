#ifndef BEAM_DECODER_BOX_HPP
#define BEAM_DECODER_BOX_HPP
#include <utility>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace Codecs {

  /** Provides a generic interface to an arbitrary Decoder */
  class DecoderBox {
    public:

      /**
       * Constructs a DecoderBox of a specified type using emplacement.
       * @param <T> The type of decoder to emplace.
       * @param args The arguments to pass to the emplaced decoder.
       */
      template<typename T, typename... Args>
      DecoderBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a DecoderBox by copying an existing Decoder.
       * @param decoder The decoder to copy.
       */
      template<typename Decoder>
      DecoderBox(Decoder decoder);

      std::size_t Decode(const void* source, std::size_t sourceSize,
        void* destination, std::size_t destinationSize);

      std::size_t Decode(const IO::SharedBuffer& source, void* destination,
        std::size_t destinationSize);

      std::size_t Decode(const void* source, std::size_t sourceSize,
        Out<IO::SharedBuffer> destination);

      std::size_t Decode(const IO::SharedBuffer& source,
        Out<IO::SharedBuffer> destination);

    private:
      struct VirtualDecoder {
        virtual ~VirtualDecoder() = default;
        virtual std::size_t Decode(const void* source, std::size_t sourceSize,
          void* destination, std::size_t destinationSize) = 0;
        virtual std::size_t Decode(const IO::SharedBuffer& source,
          void* destination, std::size_t destinationSize) = 0;
        virtual std::size_t Decode(const void* source, std::size_t sourceSize,
          Out<IO::SharedBuffer> destination) = 0;
        virtual std::size_t Decode(const IO::SharedBuffer& source,
          Out<IO::SharedBuffer> destination) = 0;
      };
      template<typename D>
      struct WrappedDecoder final : VirtualDecoder {
        using Decoder = D;
        GetOptionalLocalPtr<Decoder> m_decoder;

        template<typename... Args>
        WrappedDecoder(Args&&... args);
        std::size_t Decode(const void* source, std::size_t sourceSize,
          void* destination, std::size_t destinationSize) override;
        std::size_t Decode(const IO::SharedBuffer& source, void* destination,
          std::size_t destinationSize) override;
        std::size_t Decode(const void* source, std::size_t sourceSize,
          Out<IO::SharedBuffer> destination) override;
        std::size_t Decode(const IO::SharedBuffer& source,
          Out<IO::SharedBuffer> destination) override;
      };
      std::unique_ptr<VirtualDecoder> m_decoder;

      DecoderBox(const DecoderBox&) = delete;
      DecoderBox& operator =(const DecoderBox&) = delete;
  };

  template<typename T, typename... Args>
  DecoderBox::DecoderBox(std::in_place_type_t<T>, Args&&... args)
    : m_decoder(std::make_unique<WrappedDecoder<T>>(
        std::forward<Args>(args)...)) {}

  template<typename Decoder>
  DecoderBox::DecoderBox(Decoder decoder)
    : DecoderBox(std::in_place_type<Decoder>, std::move(decoder)) {}

  inline std::size_t DecoderBox::Decode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    return m_decoder->Decode(source, sourceSize, destination, destinationSize);
  }

  inline std::size_t DecoderBox::Decode(const IO::SharedBuffer& source,
      void* destination, std::size_t destinationSize) {
    return m_decoder->Decode(source, destination, destinationSize);
  }

  inline std::size_t DecoderBox::Decode(const void* source,
      std::size_t sourceSize, Out<IO::SharedBuffer> destination) {
    return m_decoder->Decode(source, sourceSize, Store(destination));
  }

  inline std::size_t DecoderBox::Decode(const IO::SharedBuffer& source,
      Out<IO::SharedBuffer> destination) {
    return m_decoder->Decode(source, Store(destination));
  }

  template<typename D>
  template<typename... Args>
  DecoderBox::WrappedDecoder<D>::WrappedDecoder(Args&&... args)
    : m_decoder(std::forward<Args>(args)...) {}

  template<typename D>
  std::size_t DecoderBox::WrappedDecoder<D>::Decode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    return m_decoder->Decode(source, sourceSize, destination, destinationSize);
  }

  template<typename D>
  std::size_t DecoderBox::WrappedDecoder<D>::Decode(
      const IO::SharedBuffer& source, void* destination,
      std::size_t destinationSize) {
    return m_decoder->Decode(source, destination, destinationSize);
  }

  template<typename D>
  std::size_t DecoderBox::WrappedDecoder<D>::Decode(const void* source,
      std::size_t sourceSize, Out<IO::SharedBuffer> destination) {
    return m_decoder->Decode(source, sourceSize, Store(destination));
  }

  template<typename D>
  std::size_t DecoderBox::WrappedDecoder<D>::Decode(
      const IO::SharedBuffer& source, Out<IO::SharedBuffer> destination) {
    return m_decoder->Decode(source, Store(destination));
  }
}

  template<>
  struct ImplementsConcept<Codecs::DecoderBox, Codecs::Decoder> :
    std::true_type {};
}

#endif
