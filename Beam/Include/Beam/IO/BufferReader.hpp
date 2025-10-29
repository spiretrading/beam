#ifndef BEAM_BUFFER_READER_HPP
#define BEAM_BUFFER_READER_HPP
#include <algorithm>
#include <boost/throw_exception.hpp>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/Reader.hpp"

namespace Beam {

  /**
   * Reads data from a Buffer.
   * @tparam B The type of Buffer to read from.
   */
  template<IsBuffer B>
  class BufferReader {
    public:
      using Buffer = B;

      /**
       * Constructs a BufferReader from a Buffer.
       * @param source The Buffer to read from.
       */
      explicit BufferReader(Buffer source);

      BufferReader(const BufferReader& reader);
      BufferReader(BufferReader&& reader);

      bool poll() const;
      template<IsBuffer R>
      std::size_t read(Out<R> destination, std::size_t size = -1);
      BufferReader& operator =(const BufferReader& reader);
      BufferReader& operator =(BufferReader&& reader);

    private:
      Buffer m_source;
      const char* m_cursor;
      std::size_t m_remaining_size;
  };

  template<IsBuffer B>
  BufferReader<B>::BufferReader(Buffer source)
    : m_source(std::move(source)),
      m_cursor(m_source.get_data()),
      m_remaining_size(m_source.get_size()) {}

  template<IsBuffer B>
  BufferReader<B>::BufferReader(const BufferReader& reader)
    : m_source(reader.m_source),
      m_cursor(
        m_source.get_data() + (m_source.get_size() - reader.m_remaining_size)),
      m_remaining_size(reader.m_remaining_size) {}

  template<IsBuffer B>
  BufferReader<B>::BufferReader(BufferReader&& reader)
      : m_source(std::move(reader.m_source)),
        m_cursor(m_source.get_data() +
          (m_source.get_size() - reader.m_remaining_size)),
        m_remaining_size(reader.m_remaining_size) {
    reader.m_cursor = nullptr;
    reader.m_remaining_size = 0;
  }

  template<IsBuffer B>
  bool BufferReader<B>::poll() const {
    return m_remaining_size != 0;
  }

  template<IsBuffer B>
  template<IsBuffer R>
  std::size_t BufferReader<B>::read(Out<R> destination, std::size_t size) {
    if(m_remaining_size == 0 || !m_cursor) {
      boost::throw_with_location(EndOfFileException());
    }
    auto read_size =
      append_up_to(*destination, m_cursor, std::min(size, m_remaining_size));
    m_cursor += read_size;
    m_remaining_size -= read_size;
    return read_size;
  }

  template<IsBuffer B>
  BufferReader<B>& BufferReader<B>::operator =(const BufferReader& reader) {
    m_source = reader.m_source;
    m_remaining_size = reader.m_remaining_size;
    m_cursor = m_source.get_data() + (m_source.get_size() - m_remaining_size);
    return *this;
  }

  template<IsBuffer B>
  BufferReader<B>& BufferReader<B>::operator =(BufferReader&& reader) {
    if(this == &reader) {
      return *this;
    }
    m_source = std::move(reader.m_source);
    m_remaining_size = reader.m_remaining_size;
    m_cursor = m_source.get_data() + (m_source.get_size() - m_remaining_size);
    reader.m_cursor = nullptr;
    reader.m_remaining_size = 0;
    return *this;
  }
}

#endif
