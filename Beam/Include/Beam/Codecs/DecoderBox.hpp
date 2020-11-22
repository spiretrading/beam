#ifndef BEAM_DECODER_BOX_HPP
#define BEAM_DECODER_BOX_HPP
#include <utility>
#include "Beam/Codecs/Codecs.hpp"
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/IO/BufferBox.hpp"
#include "Beam/IO/BufferView.hpp"
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
      explicit DecoderBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a DecoderBox by copying an existing Decoder.
       * @param decoder The decoder to copy.
       */
      template<typename Decoder>
      explicit DecoderBox(Decoder decoder);

      DecoderBox(const DecoderBox&) = default;

      std::size_t Decode(const void* source, std::size_t sourceSize,
        void* destination, std::size_t destinationSize);

      template<typename Buffer>
      std::size_t Decode(const Buffer& source, void* destination,
        std::size_t destinationSize);

      template<typename Buffer>
      std::size_t Decode(const void* source, std::size_t sourceSize,
        Out<Buffer> destination);

      template<typename SourceBuffer, typename Buffer>
      std::size_t Decode(const SourceBuffer& source, Out<Buffer> destination);

      DecoderBox& operator =(const DecoderBox&) = default;

    private:
      struct VirtualDecoder {
        virtual ~VirtualDecoder() = default;
        virtual std::size_t Decode(const void* source, std::size_t sourceSize,
          void* destination, std::size_t destinationSize) = 0;
        virtual std::size_t Decode(const IO::BufferView& source,
          void* destination, std::size_t destinationSize) = 0;
        virtual std::size_t Decode(const void* source, std::size_t sourceSize,
          Out<IO::BufferBox> destination) = 0;
        virtual std::size_t Decode(const IO::BufferView& source,
          Out<IO::BufferBox> destination) = 0;
      };
      template<typename D>
      struct WrappedDecoder final : VirtualDecoder {
        using Decoder = D;
        GetOptionalLocalPtr<Decoder> m_decoder;

        template<typename... Args>
        WrappedDecoder(Args&&... args);
        std::size_t Decode(const void* source, std::size_t sourceSize,
          void* destination, std::size_t destinationSize) override;
        std::size_t Decode(const IO::BufferView& source, void* destination,
          std::size_t destinationSize) override;
        std::size_t Decode(const void* source, std::size_t sourceSize,
          Out<IO::BufferBox> destination) override;
        std::size_t Decode(const IO::BufferView& source,
          Out<IO::BufferBox> destination) override;
      };
      std::shared_ptr<VirtualDecoder> m_decoder;
  };

  template<typename T, typename... Args>
  DecoderBox::DecoderBox(std::in_place_type_t<T>, Args&&... args)
    : m_decoder(std::make_shared<WrappedDecoder<T>>(
        std::forward<Args>(args)...)) {}

  template<typename Decoder>
  DecoderBox::DecoderBox(Decoder decoder)
    : DecoderBox(std::in_place_type<Decoder>, std::move(decoder)) {}

  inline std::size_t DecoderBox::Decode(const void* source,
      std::size_t sourceSize, void* destination, std::size_t destinationSize) {
    return m_decoder->Decode(source, sourceSize, destination, destinationSize);
  }

  template<typename Buffer>
  std::size_t DecoderBox::Decode(const Buffer& source, void* destination,
      std::size_t destinationSize) {
    return m_decoder->Decode(source, destination, destinationSize);
  }

  template<typename Buffer>
  std::size_t DecoderBox::Decode(const void* source, std::size_t sourceSize,
      Out<Buffer> destination) {
    auto box = IO::BufferBox(&*destination);
    return m_decoder->Decode(source, sourceSize, Store(box));
  }

  template<typename SourceBuffer, typename DestinationBuffer>
  inline std::size_t DecoderBox::Decode(const SourceBuffer& source,
      Out<DestinationBuffer> destination) {
    auto box = IO::BufferBox(&*destination);
    return m_decoder->Decode(source, Store(box));
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
      const IO::BufferView& source, void* destination,
      std::size_t destinationSize) {
    return m_decoder->Decode(source, destination, destinationSize);
  }

  template<typename D>
  std::size_t DecoderBox::WrappedDecoder<D>::Decode(const void* source,
      std::size_t sourceSize, Out<IO::BufferBox> destination) {
    return m_decoder->Decode(source, sourceSize, Store(destination));
  }

  template<typename D>
  std::size_t DecoderBox::WrappedDecoder<D>::Decode(
      const IO::BufferView& source, Out<IO::BufferBox> destination) {
    return m_decoder->Decode(source, Store(destination));
  }
}

  template<>
  struct ImplementsConcept<Codecs::DecoderBox, Codecs::Decoder> :
    std::true_type {};
}

#endif
