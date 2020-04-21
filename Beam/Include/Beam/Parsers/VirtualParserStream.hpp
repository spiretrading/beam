#ifndef BEAM_VIRTUALPARSERSTREAM_HPP
#define BEAM_VIRTUALPARSERSTREAM_HPP
#include <cstddef>
#include "Beam/Parsers/Parsers.hpp"
#include "Beam/Parsers/ParserStream.hpp"

namespace Beam::Parsers {

  /*! \class VirtualParserStream
      \brief Implements a ParserStream using virtual methods.
   */
  class VirtualParserStream {
    public:
      virtual ~VirtualParserStream() = default;

      virtual char GetChar() const = 0;

      virtual bool Read() = 0;

      virtual void Undo() = 0;

      virtual void Undo(std::size_t count) = 0;

      virtual void Accept() = 0;
  };

  template<typename S>
  class WrapperParserStream : public VirtualParserStream {
    public:
      using Stream = S;

      WrapperParserStream(Stream& stream);

      char GetChar() const override;

      bool Read() override;

      void Undo() override;

      void Undo(std::size_t count) override;

      void Accept() override;

    private:
      Stream* m_stream;
  };

  template<typename S>
  WrapperParserStream<S>::WrapperParserStream(Stream& stream)
    : m_stream(&stream) {}

  template<typename S>
  char WrapperParserStream<S>::GetChar() const {
    return m_stream->GetChar();
  }

  template<typename S>
  bool WrapperParserStream<S>::Read() {
    return m_stream->Read();
  }

  template<typename S>
  void WrapperParserStream<S>::Undo() {
    m_stream->Undo();
  }

  template<typename S>
  void WrapperParserStream<S>::Undo(std::size_t count) {
    m_stream->Undo(count);
  }

  template<typename S>
  void WrapperParserStream<S>::Accept() {
    m_stream->Accept();
  }
}

#endif
