#ifndef BEAM_BUFFER_READER_HPP
#define BEAM_BUFFER_READER_HPP
#include <cstring>
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace IO {

  /**
   * Reads data from a Buffer.
   * @param <B> The type of Buffer to read from.
   */
  template<typename B>
  class BufferReader {
    public:
      using Buffer = B;

      /**
       * Constructs a BufferReader from a Buffer.
       * @param source The Buffer to read from.
       */
      template<typename BF>
      BufferReader(BF&& source);

      /**
       * Copies a BufferReader.
       * @param reader The BufferReader to copy.
       */
      BufferReader(const BufferReader& reader);

      /**
       * Acquires ownership of a BufferReader.
       * @param reader The Reader to acquire.
       */
      BufferReader(BufferReader&& reader);

      bool IsDataAvailable() const;

      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      std::size_t Read(Out<Buffer> destination, std::size_t size);

      BufferReader& operator =(const BufferReader& reader);

      BufferReader& operator =(BufferReader&& reader);

    private:
      Buffer m_source;
      const char* m_readIterator;
      std::size_t m_readRemaining;
  };

  template<typename BF>
  BufferReader(BF&& source) -> BufferReader<std::decay_t<BF>>;

  template<typename B>
  template<typename BF>
  BufferReader<B>::BufferReader(BF&& source)
    : m_source(std::forward<BF>(source)),
      m_readIterator(m_source.GetData()),
      m_readRemaining(m_source.GetSize()) {}

  template<typename B>
  BufferReader<B>::BufferReader(const BufferReader& reader)
    : m_source(reader.m_source),
      m_readIterator(m_source.GetData() +
        (m_source.GetSize() - reader.m_readRemaining)),
      m_readRemaining(reader.m_readRemaining) {}

  template<typename B>
  BufferReader<B>::BufferReader(BufferReader&& reader)
      : m_source(std::move(reader.m_source)),
        m_readIterator(m_source.GetData() +
          (m_source.GetSize() - reader.m_readRemaining)),
        m_readRemaining(reader.m_readRemaining) {
    reader.m_readIterator = nullptr;
    reader.m_readRemaining = 0;
  }

  template<typename B>
  bool BufferReader<B>::IsDataAvailable() const {
    return m_readRemaining > 0;
  }

  template<typename B>
  std::size_t BufferReader<B>::Read(Out<Buffer> destination) {
    if(m_readRemaining == 0) {
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    destination->Append(m_readIterator, m_readRemaining);
    auto result = m_readRemaining;
    m_readRemaining = 0;
    return result;
  }

  template<typename B>
  std::size_t BufferReader<B>::Read(char* destination, std::size_t size) {
    if(m_readRemaining == 0) {
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    auto result = std::min(size, m_readRemaining);
    std::memcpy(destination, m_readIterator, result);
    m_readRemaining -= result;
    m_readIterator += result;
    return result;
  }

  template<typename B>
  std::size_t BufferReader<B>::Read(Out<Buffer> destination, std::size_t size) {
    if(m_readRemaining == 0) {
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    auto result = std::min(m_readRemaining, size);
    destination->Append(m_readIterator, result);
    m_readIterator += result;
    m_readRemaining -= result;
    return result;
  }

  template<typename B>
  BufferReader<B>& BufferReader<B>::operator =(const BufferReader& reader) {
    if(this == &reader) {
      return *this;
    }
    m_source = reader.m_source;
    m_readIterator = m_source.GetData() +
      (m_source.GetSize() - reader.m_readRemaining);
    m_readRemaining = reader.m_readRemaining;
    return *this;
  }

  template<typename B>
  BufferReader<B>& BufferReader<B>::operator =(BufferReader&& reader) {
    if(this == &reader) {
      return *this;
    }
    m_source = std::move(reader.m_source);
    m_readIterator = m_source.GetData() +
      (m_source.GetSize() - reader.m_readRemaining);
    m_readRemaining = reader.m_readRemaining;
    reader.m_readIterator = nullptr;
    reader.m_readRemaining = 0;
    return *this;
  }
}

  template<typename B>
  struct ImplementsConcept<IO::BufferReader<B>, IO::Reader<B>> :
    std::true_type {};
}

#endif
