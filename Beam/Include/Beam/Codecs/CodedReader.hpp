#ifndef BEAM_CODEDREADER_HPP
#define BEAM_CODEDREADER_HPP
#include <limits>
#include <boost/noncopyable.hpp>
#include "Beam/Codecs/Decoder.hpp"
#include "Beam/IO/PipedReader.hpp"
#include "Beam/IO/PipedWriter.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace Codecs {

  /*! \class CodedReader
      \brief Reads coded data using a Decoder.
      \tparam BufferType The type of Buffer to read into.
      \tparam SourceReaderType The type of Reader to read encoded data from.
      \tparam DecoderType The type of Decoder used.
   */
  template<typename BufferType, typename SourceReaderType, typename DecoderType>
  class CodedReader : private boost::noncopyable {
    public:

      //! The type of Buffer to read into.
      using Buffer = BufferType;

      //! The type of Reader to read from.
      using SourceReader = GetTryDereferenceType<SourceReaderType>;

      //! The type of Decoder used.
      using Decoder = GetTryDereferenceType<DecoderType>;

      //! Constructs a CodedReader.
      /*!
        \param source Initializes the source to read from.
        \param decoder Initializes the Decoder to use.
      */
      template<typename SourceReaderForward, typename DecoderForward>
      CodedReader(SourceReaderForward&& source, DecoderForward&& decoder);

      bool IsDataAvailable() const;

      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      GetOptionalLocalPtr<SourceReaderType> m_source;
      GetOptionalLocalPtr<DecoderType> m_decoder;
      IO::PipedReader<Buffer> m_reader;
      IO::PipedWriter<Buffer> m_writer;
      Buffer m_sourceBuffer;
      Buffer m_decoderBuffer;

      void ReadSource();
  };

  template<typename BufferType, typename SourceReader, typename DecoderType>
  template<typename SourceReaderForward, typename DecoderForward>
  CodedReader<BufferType, SourceReader, DecoderType>::CodedReader(
      SourceReaderForward&& source, DecoderForward&& decoder)
      : m_source(std::forward<SourceReaderForward>(source)),
        m_decoder(std::forward<DecoderForward>(decoder)),
        m_writer(Ref(m_reader)) {}

  template<typename BufferType, typename SourceReader, typename DecoderType>
  bool CodedReader<BufferType, SourceReader, DecoderType>::
      IsDataAvailable() const {
    return m_reader.IsDataAvailable() || m_source->IsDataAvailable();
  }

  template<typename BufferType, typename SourceReader, typename DecoderType>
  std::size_t CodedReader<BufferType, SourceReader, DecoderType>::Read(
      Out<Buffer> destination) {
    return Read(Store(destination), std::numeric_limits<std::size_t>::max());
  }

  template<typename BufferType, typename SourceReader, typename DecoderType>
  std::size_t CodedReader<BufferType, SourceReader, DecoderType>::Read(
      char* destination, std::size_t size) {
    ReadSource();
    return m_reader.Read(destination, size);
  }

  template<typename BufferType, typename SourceReader, typename DecoderType>
  std::size_t CodedReader<BufferType, SourceReader, DecoderType>::Read(
      Out<Buffer> destination, std::size_t size) {
    ReadSource();
    return m_reader.Read(Store(destination), size);
  }

  template<typename BufferType, typename SourceReader, typename DecoderType>
  void CodedReader<BufferType, SourceReader, DecoderType>::ReadSource() {
    if(m_reader.IsDataAvailable()) {
      return;
    }
    try {
      m_source->Read(Store(m_sourceBuffer));
    } catch(...) {
      m_writer.Break(std::current_exception());
      return;
    }
    if(InPlaceSupport<DecoderType>::value) {
      try {
        m_decoder->Decode(m_sourceBuffer, Store(m_sourceBuffer));
      } catch(...) {
        m_writer.Break(std::current_exception());
        return;
      }
      m_writer.Write(m_sourceBuffer);
      m_sourceBuffer.Reset();
    } else {
      try {
        m_decoder->Decode(m_sourceBuffer, Store(m_decoderBuffer));
      } catch(...) {
        m_writer.Break(std::current_exception());
        return;
      }
      m_writer.Write(m_decoderBuffer);
      m_decoderBuffer.Reset();
      m_sourceBuffer.Reset();
    }
  }
}

  template<typename BufferType, typename SourceReader, typename DecoderType>
  struct ImplementsConcept<Codecs::CodedReader<BufferType, SourceReader,
    DecoderType>, IO::Reader<BufferType>> : std::true_type {};
}

#endif
