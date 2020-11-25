#ifndef BEAM_CODED_READER_HPP
#define BEAM_CODED_READER_HPP
#include <limits>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/IO/IOException.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Codecs {

  /**
   * Reads coded data using a Decoder.
   * @param <B> The type of Buffer to read into.
   * @param <R> The type of Reader to read encoded data from.
   * @param <D> The type of Decoder used.
   */
  template<typename B, typename R, typename D>
  class CodedReader {
    public:

      /** The type of Buffer to read into. */
      using Buffer = B;

      /** The type of Reader to read from. */
      using SourceReader = GetTryDereferenceType<R>;

      /** The type of Decoder used. */
      using Decoder = GetTryDereferenceType<D>;

      /**
       * Constructs a CodedReader.
       * @param source Initializes the source to read from.
       * @param decoder Initializes the Decoder to use.
       */
      template<typename SF, typename DF>
      CodedReader(SF&& source, DF&& decoder);

      bool IsDataAvailable() const;

      template<typename T>
      std::size_t Read(Out<T> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename T>
      std::size_t Read(Out<T> destination, std::size_t size);

    private:
      GetOptionalLocalPtr<R> m_source;
      GetOptionalLocalPtr<D> m_decoder;
      IO::PipedReader<Buffer> m_reader;
      IO::PipedWriter<Buffer> m_writer;
      Buffer m_sourceBuffer;
      Buffer m_decoderBuffer;

      CodedReader(const CodedReader&) = delete;
      CodedReader& operator =(const CodedReader&) = delete;
      void ReadSource();
  };

  template<typename B, typename R, typename D>
  template<typename SF, typename DF>
  CodedReader<B, R, D>::CodedReader(SF&& source, DF&& decoder)
    : m_source(std::forward<SF>(source)),
      m_decoder(std::forward<DF>(decoder)),
      m_writer(Ref(m_reader)) {}

  template<typename B, typename R, typename D>
  bool CodedReader<B, R, D>::IsDataAvailable() const {
    return m_reader.IsDataAvailable() || m_source->IsDataAvailable();
  }

  template<typename B, typename R, typename D>
  template<typename T>
  std::size_t CodedReader<B, R, D>::Read(Out<T> destination) {
    return Read(Store(destination), std::numeric_limits<std::size_t>::max());
  }

  template<typename B, typename R, typename D>
  std::size_t CodedReader<B, R, D>::Read(char* destination, std::size_t size) {
    ReadSource();
    return m_reader.Read(destination, size);
  }

  template<typename B, typename R, typename D>
  template<typename T>
  std::size_t CodedReader<B, R, D>::Read(Out<T> destination, std::size_t size) {
    ReadSource();
    return m_reader.Read(Store(destination), size);
  }

  template<typename B, typename R, typename D>
  void CodedReader<B, R, D>::ReadSource() {
    if(m_reader.IsDataAvailable()) {
      return;
    }
    try {
      m_source->Read(Store(m_sourceBuffer));
    } catch(const std::exception&) {
      m_writer.Break(std::current_exception());
      return;
    }
    if constexpr(InPlaceSupport<Decoder>::value) {
      try {
        m_decoder->Decode(m_sourceBuffer, Store(m_sourceBuffer));
      } catch(const std::exception&) {
        m_writer.Break(NestCurrentException(
          IO::IOException("Decoder failed.")));
        return;
      }
      m_writer.Write(m_sourceBuffer);
      m_sourceBuffer.Reset();
    } else {
      try {
        m_decoder->Decode(m_sourceBuffer, Store(m_decoderBuffer));
      } catch(const std::exception&) {
        m_writer.Break(NestCurrentException(
          IO::IOException("Decoder failed.")));
        return;
      }
      m_writer.Write(m_decoderBuffer);
      m_decoderBuffer.Reset();
      m_sourceBuffer.Reset();
    }
  }
}

  template<typename B, typename R, typename D>
  struct ImplementsConcept<Codecs::CodedReader<B, R, D>, IO::Reader> :
    std::true_type {};
}

#endif
