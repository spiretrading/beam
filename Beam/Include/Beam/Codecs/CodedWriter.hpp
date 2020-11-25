#ifndef BEAM_CODED_WRITER_HPP
#define BEAM_CODED_WRITER_HPP
#include "Beam/Codecs/Encoder.hpp"
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace Codecs {

  /**
   * Writes coded data using a Decoder.
   * @param <W> The type of Writer to write the encoded data to.
   * @param <E> The type of Encoder to use.
   */
  template<typename W, typename E>
  class CodedWriter {
    public:

      /** The type of Writer to write the encoded data to. */
      using DestinationWriter = GetTryDereferenceType<W>;

      /** The type of Buffer used for encoding. */
      using Buffer = typename DestinationWriter::Buffer;

      /** The type of Encoder to use. */
      using Encoder = GetTryDereferenceType<E>;

      /**
       * Constructs a CodedWriter.
       * @param destination The destination to write to.
       * @param encoder The Encoder to use.
       */
      template<typename WF, typename EF>
      CodedWriter(WF&& destination, EF&& encoder);

      void Write(const void* data, std::size_t size);

      template<typename B>
      void Write(const B& data);

    private:
      GetOptionalLocalPtr<W> m_destination;
      GetOptionalLocalPtr<E> m_encoder;

      CodedWriter(const CodedWriter&) = delete;
      CodedWriter& operator =(const CodedWriter&) = delete;
  };

  template<typename W, typename E>
  template<typename WF, typename EF>
  CodedWriter<W, E>::CodedWriter(WF&& destination, EF&& encoder)
    : m_destination(std::forward<WF>(destination)),
      m_encoder(std::forward<EF>(encoder)) {}

  template<typename W, typename E>
  void CodedWriter<W, E>::Write(const void* data, std::size_t size) {
    auto encodedData = Buffer();
    try {
      m_encoder->Encode(data, size, Store(encodedData));
    } catch(const std::exception&) {
      std::throw_with_nested(IO::IOException("Encoder failed."));
    }
    m_destination->Write(encodedData);
  }

  template<typename W, typename E>
  template<typename B>
  void CodedWriter<W, E>::Write(const B& data) {
    Write(data.GetData(), data.GetSize());
  }
}

  template<typename D, typename E>
  struct ImplementsConcept<Codecs::CodedWriter<D, E>,
    IO::Writer<typename Codecs::CodedWriter<D, E>::Buffer>> : std::true_type {};
}

#endif
