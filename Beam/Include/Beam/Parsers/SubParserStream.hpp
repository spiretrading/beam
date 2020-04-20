#ifndef BEAM_SUB_PARSER_STREAM_HPP
#define BEAM_SUB_PARSER_STREAM_HPP
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserStream.hpp"

namespace Beam::Parsers {

  /**
   * Implements a ParserStream that wraps an existing ParserStream, useful
   * mostly as a way to implement backtracking.
   * @param <P> The ParserStream to wrap.
   */
  template<typename P>
  class SubParserStream {
    public:

      /** The ParserStream to wrap. */
      using ParserStream = P;

      /**
       * Constructs a SubParserStream.
       * @param stream The ParserStream to wrap.
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

      SubParserStream(const SubParserStream&) = delete;
      SubParserStream(SubParserStream&) = delete;
  };

  template<typename P>
  SubParserStream<P>::SubParserStream(ParserStream& stream)
    : m_stream(&stream),
      m_sizeRead(0) {}

  template<typename P>
  SubParserStream<P>::~SubParserStream() {
    m_stream->Undo(m_sizeRead);
  }

  template<typename P>
  char SubParserStream<P>::GetChar() const {
    return m_stream->GetChar();
  }

  template<typename P>
  bool SubParserStream<P>::Read() {
    if(!m_stream->Read()) {
      return false;
    }
    ++m_sizeRead;
    return true;
  }

  template<typename P>
  void SubParserStream<P>::Undo() {
    Undo(1);
  }

  template<typename P>
  void SubParserStream<P>::Undo(std::size_t count) {
    m_sizeRead -= count;
    m_stream->Undo(count);
  }

  template<typename P>
  void SubParserStream<P>::Accept() {
    m_sizeRead = 0;
  }
}

#endif
