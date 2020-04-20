#ifndef BEAM_READERPARSERSTREAM_HPP
#define BEAM_READERPARSERSTREAM_HPP
#include <utility>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserStream.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam::Parsers {

  /*! \class ReaderParserStream
      \brief Implements the ParserStream using a Reader as the data source.
      \tparam R The Reader to use as the data source.
  */
  template<typename R>
  class ReaderParserStream {
    public:

      //! The Reader to use as the data source.
      using Reader = GetTryDereferenceType<R>;

      //! Constructs a ReaderParserStream.
      /*!
        \param source Initializes the Reader used as the data source.
      */
      template<typename RF>
      ReaderParserStream(RF&& source);

      char GetChar() const;

      bool Read();

      void Undo();

      void Undo(std::size_t count);

      void Accept();

    private:
      GetOptionalLocalPtr<R> m_source;
      typename Reader::Buffer m_buffer;
      const char* m_position;
      std::size_t m_sizeRemaining;
  };

  template<typename R>
  ReaderParserStream(R&& source) -> ReaderParserStream<std::decay_t<R>>;

  inline auto ParserStreamFromString(const std::string& source) {
    return ReaderParserStream(
      IO::BufferReader(IO::BufferFromString<IO::SharedBuffer>(source)));
  }

  template<typename R>
  template<typename RF>
  ReaderParserStream<R>::ReaderParserStream(RF&& source)
    : m_source(std::forward<RF>(source)),
      m_position(m_buffer.GetData()),
      m_sizeRemaining(0) {}

  template<typename R>
  char ReaderParserStream<R>::GetChar() const {
    return *m_position;
  }

  template<typename R>
  bool ReaderParserStream<R>::Read() {
    constexpr auto READ_SIZE = std::size_t(1024);
    if(m_sizeRemaining == 0) {
      auto buffer = typename Reader::Buffer();
      auto position = m_position - m_buffer.GetData();
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

  template<typename R>
  void ReaderParserStream<R>::Undo() {
    Undo(1);
  }

  template<typename R>
  void ReaderParserStream<R>::Undo(std::size_t count) {
    m_sizeRemaining += count;
    m_position -= count;
  }

  template<typename R>
  void ReaderParserStream<R>::Accept() {
    if(m_sizeRemaining == 0) {
      m_buffer.Reset();
    }
  }
}

#endif
