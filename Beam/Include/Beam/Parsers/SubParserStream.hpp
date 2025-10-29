#ifndef BEAM_SUB_PARSER_STREAM_HPP
#define BEAM_SUB_PARSER_STREAM_HPP
#include "Beam/Parsers/ParserStream.hpp"

namespace Beam {

  /**
   * Wraps an existing ParserStream to provide a local view suitable for
   * backtracking.
   * @tparam P The ParserStream type to wrap.
   */
  template<IsParserStream P>
  class SubParserStream {
    public:

      /** The type of ParserStream being wrapped. */
      using ParserStream = P;

      /**
       * Constructs a SubParserStream that wraps the specified parser stream.
       * @param stream The ParserStream to wrap.
       */
      explicit SubParserStream(ParserStream& stream);

      ~SubParserStream();

      char peek() const;
      bool read();
      void undo();
      void undo(std::size_t count);
      void accept();

    private:
      ParserStream* m_stream;
      std::size_t m_count;

      SubParserStream(const SubParserStream&) = delete;
      SubParserStream& operator =(const SubParserStream&) = delete;
  };

  template<IsParserStream P>
  SubParserStream<P>::SubParserStream(ParserStream& stream)
    : m_stream(&stream),
      m_count(0) {}

  template<IsParserStream P>
  SubParserStream<P>::~SubParserStream() {
    m_stream->undo(m_count);
  }

  template<IsParserStream P>
  char SubParserStream<P>::peek() const {
    return m_stream->peek();
  }

  template<IsParserStream P>
  bool SubParserStream<P>::read() {
    if(!m_stream->read()) {
      return false;
    }
    ++m_count;
    return true;
  }

  template<IsParserStream P>
  void SubParserStream<P>::undo() {
    undo(1);
  }

  template<IsParserStream P>
  void SubParserStream<P>::undo(std::size_t count) {
    m_count -= count;
    m_stream->undo(count);
  }

  template<IsParserStream P>
  void SubParserStream<P>::accept() {
    m_count = 0;
  }
}

#endif
