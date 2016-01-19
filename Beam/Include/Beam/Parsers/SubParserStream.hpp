#ifndef BEAM_SUBPARSERSTREAM_HPP
#define BEAM_SUBPARSERSTREAM_HPP
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class SubParserStream
      \brief Implements a ParserStream that wraps an existing ParserStream,
             useful mostly as a way to implement backtracking.
      \tparam ParserStreamType The ParserStream to wrap.
  */
  template<typename ParserStreamType>
  class SubParserStream {
    public:

      //! The ParserStream to wrap.
      typedef ParserStreamType ParserStream;

      //! Constructs a SubParserStream.
      /*!
        \param stream The ParserStream to wrap.
      */
      SubParserStream(ParserStream& stream);

      ~SubParserStream();

      char GetChar() const;

      bool Read();

      void Undo();

      void Undo(std::size_t count);

      void Accept();

    private:
      ParserStream* m_stream;
      std::size_t m_sizeRead;
  };

  template<typename ParserStreamType>
  SubParserStream<ParserStreamType>::SubParserStream(ParserStream& stream)
      : m_stream(&stream),
        m_sizeRead(0) {}

  template<typename ParserStreamType>
  SubParserStream<ParserStreamType>::~SubParserStream() {
    m_stream->Undo(m_sizeRead);
  }

  template<typename ParserStreamType>
  char SubParserStream<ParserStreamType>::GetChar() const {
    return m_stream->GetChar();
  }

  template<typename ParserStreamType>
  bool SubParserStream<ParserStreamType>::Read() {
    if(!m_stream->Read()) {
      return false;
    }
    ++m_sizeRead;
    return true;
  }

  template<typename ParserStreamType>
  void SubParserStream<ParserStreamType>::Undo() {
    Undo(1);
  }

  template<typename ParserStreamType>
  void SubParserStream<ParserStreamType>::Undo(std::size_t count) {
    m_sizeRead -= count;
    m_stream->Undo(count);
  }

  template<typename ParserStreamType>
  void SubParserStream<ParserStreamType>::Accept() {
    m_sizeRead = 0;
  }
}
}

#endif
