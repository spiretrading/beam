#ifndef BEAM_PARSER_STREAM_HPP
#define BEAM_PARSER_STREAM_HPP
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Pointers/VirtualPtr.hpp"
#include "Beam/Utilities/TypeTraits.hpp"

namespace Beam {

  /** Concept for parser streams. */
  template<typename T>
  concept IsParserStream = requires(T& t, const T& ct, std::size_t count) {
    { ct.peek() } -> std::convertible_to<char>;
    { t.read() } -> std::convertible_to<bool>;
    { t.undo() };
    { t.undo(count) };
    { t.accept() };
  };

  /**
   * A parser stream is an object that can be used to read characters, put
   * characters back into the stream, and flush the stream's buffer.
   */
  class ParserStream {
    public:

      /**
       * Constructs a ParserStream of a specified type using emplacement.
       * @tparam T The type of parser stream to emplace.
       * @param args The arguments to pass to the emplaced parser stream.
       */
      template<IsParserStream T, typename... Args>
      explicit ParserStream(std::in_place_type_t<T>, Args&&... args);

      /**
       * Constructs a ParserStream by referencing an existing parser stream.
       * @param parser_stream The parser stream to reference.
       */
      template<DisableCopy<ParserStream> T> requires
        IsParserStream<dereference_t<T>>
      ParserStream(T&& parser_stream);

      ParserStream(const ParserStream&) = default;
      ParserStream(ParserStream&&) = default;

      /** Returns the last character read from the stream. */
      char peek() const;

      /**
       * Reads the next character in the stream.
       * @return true iff a character was successfully read from the stream.
       */
      bool read();

      /** Puts back the last character read. */
      void undo();

      /**
       * Puts back multiple characters last read.
       * @param count The number of characters to put back.
       */
      void undo(std::size_t count);

      /** Used to flush the stream's buffer. */
      void accept();

    private:
      struct VirtualParserStream {
        virtual ~VirtualParserStream() = default;

        virtual char peek() const = 0;
        virtual bool read() = 0;
        virtual void undo() = 0;
        virtual void undo(std::size_t) = 0;
        virtual void accept() = 0;
      };
      template<typename P>
      struct WrappedParserStream final : VirtualParserStream {
        using Parser = P;
        local_ptr_t<P> m_parser_stream;

        template<typename... Args>
        WrappedParserStream(Args&&... args);

        char peek() const override;
        bool read() override;
        void undo() override;
        void undo(std::size_t count) override;
        void accept() override;
      };
      VirtualPtr<VirtualParserStream> m_parser_stream;
  };

  template<IsParserStream P, typename... Args>
  ParserStream::ParserStream(std::in_place_type_t<P>, Args&&... args)
    : m_parser_stream(make_virtual_ptr<WrappedParserStream<P>>(
        std::forward<Args>(args)...)) {}

  template<DisableCopy<ParserStream> T> requires
    IsParserStream<dereference_t<T>>
  ParserStream::ParserStream(T&& parser_stream)
    : m_parser_stream(make_virtual_ptr<WrappedParserStream<
        std::remove_cvref_t<T>>>(std::forward<T>(parser_stream))) {}

  inline char ParserStream::peek() const {
    return m_parser_stream->peek();
  }

  inline bool ParserStream::read() {
    return m_parser_stream->read();
  }

  inline void ParserStream::undo() {
    m_parser_stream->undo();
  }

  inline void ParserStream::undo(std::size_t count) {
    m_parser_stream->undo(count);
  }

  inline void ParserStream::accept() {
    m_parser_stream->accept();
  }

  template<typename P>
  template<typename... Args>
  ParserStream::WrappedParserStream<P>::WrappedParserStream(Args&&... args)
    : m_parser_stream(std::forward<Args>(args)...) {}

  template<typename P>
  char ParserStream::WrappedParserStream<P>::peek() const {
    return m_parser_stream->peek();
  }

  template<typename P>
  bool ParserStream::WrappedParserStream<P>::read() {
    return m_parser_stream->read();
  }

  template<typename P>
  void ParserStream::WrappedParserStream<P>::undo() {
    m_parser_stream->undo();
  }

  template<typename P>
  void ParserStream::WrappedParserStream<P>::undo(std::size_t count) {
    m_parser_stream->undo(count);
  }

  template<typename P>
  void ParserStream::WrappedParserStream<P>::accept() {
    m_parser_stream->accept();
  }
}

#endif
