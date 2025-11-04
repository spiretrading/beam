#ifndef BEAM_READER_PARSER_STREAM_HPP
#define BEAM_READER_PARSER_STREAM_HPP
#include <string_view>
#include <utility>
#include "Beam/IO/BufferReader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Parsers/ParserStream.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {

  /**
   * Adapts a Reader to the ParserStream interface.
   * @tparam R The Reader to adapt.
   */
  template<typename R> requires IsReader<dereference_t<R>>
  class ReaderParserStream {
    public:

      /** The Reader being adapted. */
      using Reader = dereference_t<R>;

      /**
       * Constructs a ReaderParserStream.
       * @param source Initializes the Reader used as the data source.
       */
      template<Initializes<R> RF>
      explicit ReaderParserStream(RF&& source);

      char peek() const;
      bool read();
      void undo();
      void undo(std::size_t count);
      void accept();

    private:
      local_ptr_t<R> m_source;
      SharedBuffer m_buffer;
      const char* m_position;
      std::size_t m_size_remaining;
  };

  template<typename R>
  ReaderParserStream(R&& source) -> ReaderParserStream<std::remove_cvref_t<R>>;

  /** Adapts a std::string_view to the ParserStream interface. */
  inline auto to_parser_stream(std::string_view source) {
    return ReaderParserStream(BufferReader(from<SharedBuffer>(source)));
  }

  /** Adapts a ConstBuffer to the ParserStream interface. */
  template<IsConstBuffer B>
  auto to_parser_stream(B&& source) {
    return ReaderParserStream(BufferReader(std::forward<B>(source)));
  }

  template<typename R> requires IsReader<dereference_t<R>>
  template<Initializes<R> RF>
  ReaderParserStream<R>::ReaderParserStream(RF&& source)
    : m_source(std::forward<RF>(source)),
      m_position(m_buffer.get_data()),
      m_size_remaining(0) {}

  template<typename R> requires IsReader<dereference_t<R>>
  char ReaderParserStream<R>::peek() const {
    return *m_position;
  }

  template<typename R> requires IsReader<dereference_t<R>>
  bool ReaderParserStream<R>::read() {
    if(m_size_remaining == 0) {
      if(m_position) {
        m_position = nullptr;
        return false;
      }
      auto buffer = SharedBuffer();
      auto position = m_buffer.get_size();
      try {
        auto read_size = m_source->read(out(buffer));
        m_size_remaining = read_size - 1;
      } catch(const EndOfFileException&) {
        return false;
      }
      append(m_buffer, std::move(buffer));
      m_position = m_buffer.get_data() + position;
      return true;
    }
    --m_size_remaining;
    ++m_position;
    return true;
  }

  template<typename R> requires IsReader<dereference_t<R>>
  void ReaderParserStream<R>::undo() {
    undo(1);
  }

  template<typename R> requires IsReader<dereference_t<R>>
  void ReaderParserStream<R>::undo(std::size_t count) {
    m_size_remaining += count;
    if(!m_position) {
      if(count != 0) {
        m_position =
          m_buffer.get_data() + m_buffer.get_size() - m_size_remaining - 1;
      }
    } else {
      m_position -= count;
    }
  }

  template<typename R> requires IsReader<dereference_t<R>>
  void ReaderParserStream<R>::accept() {
    if(m_size_remaining == 0) {
      reset(m_buffer);
    }
  }
}

#endif
