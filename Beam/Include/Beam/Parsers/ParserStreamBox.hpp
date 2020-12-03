#ifndef BEAM_PARSER_STREAM_BOX_HPP
#define BEAM_PARSER_STREAM_BOX_HPP
#include <memory>
#include <type_traits>
#include <utility>
#include "Beam/Parsers/ParserStream.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam::Parsers {

  /** Provides a generic interface over an arbitrary ParserStream. */
  class ParserStreamBox {
    public:

      /**
       * Constructs a ParserStreamBox of a specified type using emplacement.
       * @param <T> The type of parser stream to emplace.
       * @param args The arguments to pass to the emplaced parser stream.
       */
      template<typename T, typename... Args>
      explicit ParserStreamBox(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ParserStreamBox by copying an existing parser stream.
       * @param stream The parser stream to copy.
       */
      template<typename ParserStream>
      explicit ParserStreamBox(ParserStream stream);

      char GetChar() const;

      bool Read();

      void Undo();

      void Undo(std::size_t count);

      void Accept();

    private:
      struct VirtualParserStream {
        virtual ~VirtualParserStream() = default;
        virtual char GetChar() const = 0;
        virtual bool Read() = 0;
        virtual void Undo() = 0;
        virtual void Undo(std::size_t count) = 0;
        virtual void Accept() = 0;
      };
      template<typename S>
      struct WrappedParserStream final : VirtualParserStream {
        GetOptionalLocalPtr<S> m_stream;

        template<typename... Args>
        WrappedParserStream(Args&&... args);
        char GetChar() const override;
        bool Read() override;
        void Undo() override;
        void Undo(std::size_t count) override;
        void Accept() override;
      };
      std::unique_ptr<VirtualParserStream> m_stream;

      ParserStreamBox(const ParserStreamBox&) = delete;
      ParserStreamBox& operator =(const ParserStreamBox&) = delete;
  };

  template<typename T, typename... Args>
  ParserStreamBox::ParserStreamBox(std::in_place_type_t<T>, Args&&... args)
    : m_stream(std::make_unique<WrappedParserStream<T>>(
        std::forward<Args>(args)...)) {}

  template<typename ParserStream>
  ParserStreamBox::ParserStreamBox(ParserStream stream)
    : ParserStreamBox(std::in_place_type<ParserStream>, std::move(stream)) {}

  inline char ParserStreamBox::GetChar() const {
    return m_stream->GetChar();
  }

  inline bool ParserStreamBox::Read() {
    return m_stream->Read();
  }

  inline void ParserStreamBox::Undo() {
    return m_stream->Undo();
  }

  inline void ParserStreamBox::Undo(std::size_t count) {
    return m_stream->Undo(count);
  }

  inline void ParserStreamBox::Accept() {
    return m_stream->Accept();
  }

  template<typename S>
  template<typename... Args>
  ParserStreamBox::WrappedParserStream<S>::WrappedParserStream(Args&&... args)
    : m_stream(std::forward<Args>(args)...) {}

  template<typename S>
  char ParserStreamBox::WrappedParserStream<S>::GetChar() const {
    return m_stream->GetChar();
  }

  template<typename S>
  bool ParserStreamBox::WrappedParserStream<S>::Read() {
    return m_stream->Read();
  }

  template<typename S>
  void ParserStreamBox::WrappedParserStream<S>::Undo() {
    return m_stream->Undo();
  }

  template<typename S>
  void ParserStreamBox::WrappedParserStream<S>::Undo(std::size_t count) {
    return m_stream->Undo(count);
  }

  template<typename S>
  void ParserStreamBox::WrappedParserStream<S>::Accept() {
    return m_stream->Accept();
  }
}

#endif
