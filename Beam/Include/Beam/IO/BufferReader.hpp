#ifndef BEAM_BUFFERREADER_HPP
#define BEAM_BUFFERREADER_HPP
#include <cstring>
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace IO {

  /*! \class BufferReader
      \brief Reads data from a Buffer.
      \tparam BufferType The type of Buffer to read from.
   */
  template<typename BufferType>
  class BufferReader : private boost::noncopyable {
    public:
      using Buffer = BufferType;

      //! Constructs a BufferReader from a Buffer.
      /*!
        \tparam BufferForward A type compatible with BufferType.
        \param source The Buffer to read from.
      */
      template<typename BufferForward>
      BufferReader(BufferForward&& source);

      //! Copies a BufferReader.
      /*!
        \param reader The BufferReader to copy.
      */
      BufferReader(const BufferReader& reader);

      //! Acquires ownership of a BufferReader.
      /*!
        \param reader The Reader to acquire.
      */
      BufferReader(BufferReader&& reader);

      BufferReader& operator =(const BufferReader& reader);

      BufferReader& operator =(BufferReader&& reader);

      bool IsDataAvailable() const;

      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      Buffer m_source;
      const char* m_readIterator;
      std::size_t m_readRemaining;
  };

  template<typename BF>
  BufferReader(BF&& source) -> BufferReader<std::decay_t<BF>>;

  template<typename BufferType>
  template<typename BufferForward>
  BufferReader<BufferType>::BufferReader(BufferForward&& source)
      : m_source(std::forward<BufferForward>(source)),
        m_readIterator(m_source.GetData()),
        m_readRemaining(m_source.GetSize()) {}

  template<typename BufferType>
  BufferReader<BufferType>::BufferReader(const BufferReader& reader)
      : m_source(reader.m_source),
        m_readIterator(m_source.GetData() +
          (m_source.GetSize() - reader.m_readRemaining)),
        m_readRemaining(reader.m_readRemaining) {}

  template<typename BufferType>
  BufferReader<BufferType>::BufferReader(BufferReader&& reader)
      : m_source(std::move(reader.m_source)),
        m_readIterator(m_source.GetData() +
          (m_source.GetSize() - reader.m_readRemaining)),
        m_readRemaining(reader.m_readRemaining) {
    reader.m_readIterator = nullptr;
    reader.m_readRemaining = 0;
  }

  template<typename BufferType>
  BufferReader<BufferType>& BufferReader<BufferType>::operator =(
      const BufferReader& reader) {
    if(this == &reader) {
      return *this;
    }
    m_source = reader.m_source;
    m_readIterator = m_source.GetData() +
      (m_source.GetSize() - reader.m_readRemaining);
    m_readRemaining = reader.m_readRemaining;
    return *this;
  }

  template<typename BufferType>
  BufferReader<BufferType>& BufferReader<BufferType>::operator =(
      BufferReader&& reader) {
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

  template<typename BufferType>
  bool BufferReader<BufferType>::IsDataAvailable() const {
    return m_readRemaining > 0;
  }

  template<typename BufferType>
  std::size_t BufferReader<BufferType>::Read(Out<Buffer> destination) {
    if(m_readRemaining == 0) {
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    destination->Append(m_readIterator, m_readRemaining);
    std::size_t result = m_readRemaining;
    m_readRemaining = 0;
    return result;
  }

  template<typename BufferType>
  std::size_t BufferReader<BufferType>::Read(char* destination,
      std::size_t size) {
    if(m_readRemaining == 0) {
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    std::size_t result = std::min(size, m_readRemaining);
    std::memcpy(destination, m_readIterator, result);
    m_readRemaining -= result;
    m_readIterator += result;
    return result;
  }

  template<typename BufferType>
  std::size_t BufferReader<BufferType>::Read(Out<Buffer> destination,
      std::size_t size) {
    if(m_readRemaining == 0) {
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    std::size_t result = std::min(m_readRemaining, size);
    destination->Append(m_readIterator, result);
    m_readIterator += result;
    m_readRemaining -= result;
    return result;
  }
}

  template<typename BufferType>
  struct ImplementsConcept<IO::BufferReader<BufferType>,
    IO::Reader<BufferType>> : std::true_type {};
}

#endif
