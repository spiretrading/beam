#ifndef BEAM_VIRTUALPARSERSTREAM_HPP
#define BEAM_VIRTUALPARSERSTREAM_HPP
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserStream.hpp"

namespace Beam {
namespace Parsers {

  /*! \class VirtualParserStream
      \brief Implements a ParserStream using virtual methods.
  */
  class VirtualParserStream {
    public:
      virtual ~VirtualParserStream();

      virtual char GetChar() const = 0;

      virtual bool Read() = 0;

      virtual void Undo() = 0;

      virtual void Undo(std::size_t count) = 0;

      virtual void Accept() = 0;
  };

  template<typename ParserStreamType>
  class WrapperParserStream : public VirtualParserStream {
    public:
      typedef ParserStreamType ParserStream;

      WrapperParserStream(ParserStream& stream);

      virtual char GetChar() const;

      virtual bool Read();

      virtual void Undo();

      virtual void Undo(std::size_t count);

      virtual void Accept();

    private:
      ParserStream* m_stream;
  };

  inline VirtualParserStream::~VirtualParserStream() {}

  template<typename ParserStreamType>
  WrapperParserStream<ParserStreamType>::WrapperParserStream(
      ParserStream& stream)
      : m_stream(&stream) {}

  template<typename ParserStreamType>
  char WrapperParserStream<ParserStreamType>::GetChar() const {
    return m_stream->GetChar();
  }

  template<typename ParserStreamType>
  bool WrapperParserStream<ParserStreamType>::Read() {
    return m_stream->Read();
  }

  template<typename ParserStreamType>
  void WrapperParserStream<ParserStreamType>::Undo() {
    m_stream->Undo();
  }

  template<typename ParserStreamType>
  void WrapperParserStream<ParserStreamType>::Undo(std::size_t count) {
    m_stream->Undo(count);
  }

  template<typename ParserStreamType>
  void WrapperParserStream<ParserStreamType>::Accept() {
    m_stream->Accept();
  }
}
}

#endif
