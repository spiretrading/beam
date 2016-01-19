#ifndef BEAM_READERPARSERSTREAM_HPP
#define BEAM_READERPARSERSTREAM_HPP
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserStream.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace Parsers {

  /*! \class ReaderParserStream
      \brief Implements the ParserStream using a Reader as the data source.
      \tparam ReaderType The Reader to use as the data source.
  */
  template<typename ReaderType>
  class ReaderParserStream {
    public:

      //! The Reader to use as the data source.
      typedef typename TryDereferenceType<ReaderType>::type Reader;

      //! Constructs a ReaderParserStream.
      /*!
        \param source Initializes the Reader used as the data source.
      */
      template<typename ReaderForward>
      ReaderParserStream(ReaderForward&& source);

      char GetChar() const;

      bool Read();

      void Undo();

      void Undo(std::size_t count);

      void Accept();

    private:
      typename OptionalLocalPtr<ReaderType>::type m_source;
      typename Reader::Buffer m_buffer;
      const char* m_position;
      std::size_t m_sizeRemaining;
  };

  inline ReaderParserStream<IO::BufferReader<IO::SharedBuffer>>
      ParserStreamFromString(const std::string& source) {
    return IO::BufferFromString<IO::SharedBuffer>(source);
  }

  template<typename ReaderType>
  template<typename ReaderForward>
  ReaderParserStream<ReaderType>::ReaderParserStream(ReaderForward&& source)
      : m_source(std::forward<ReaderForward>(source)),
        m_position(m_buffer.GetData()),
        m_sizeRemaining(0) {}

  template<typename ReaderType>
  char ReaderParserStream<ReaderType>::GetChar() const {
    return *m_position;
  }

  template<typename ReaderType>
  bool ReaderParserStream<ReaderType>::Read() {
    static const std::size_t READ_SIZE = 1024;
    if(m_sizeRemaining == 0) {
      typename Reader::Buffer buffer;
      std::ptrdiff_t position = m_position - m_buffer.GetData();
      try {
        m_sizeRemaining = m_source->Read(Store(buffer), READ_SIZE) - 1;
      } catch(const IO::EndOfFileException&) {
        return false;
      }
      m_buffer.Append(buffer);
      m_position = m_buffer.GetData() + position;
      if(position != 0) {
        ++m_position;
      }
      return true;
    }
    --m_sizeRemaining;
    ++m_position;
    return true;
  }

  template<typename ReaderType>
  void ReaderParserStream<ReaderType>::Undo() {
    Undo(1);
  }

  template<typename ReaderType>
  void ReaderParserStream<ReaderType>::Undo(std::size_t count) {
    m_sizeRemaining += count;
    m_position -= count;
  }

  template<typename ReaderType>
  void ReaderParserStream<ReaderType>::Accept() {
    if(m_sizeRemaining == 0) {
      m_buffer.Reset();
    }
  }
}
}

#endif
