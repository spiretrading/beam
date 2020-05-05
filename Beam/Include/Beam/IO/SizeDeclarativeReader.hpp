#ifndef BEAM_SIZEDECLARATIVEREADER_HPP
#define BEAM_SIZEDECLARATIVEREADER_HPP
#include <cstdint>
#include <limits>
#include <boost/noncopyable.hpp>
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/Endian.hpp"

namespace Beam {
namespace IO {

  /*! \class SizeDeclariveReader
      \brief Reads data whose size is declared at the beginning.
      \tparam SourceReaderType The type of Reader to read from.
   */
  template<typename SourceReaderType>
  class SizeDeclarativeReader : private boost::noncopyable {
    public:
      using Buffer = SharedBuffer;

      //! The source to read from.
      using SourceReader = typename TryDereferenceType<SourceReaderType>::type;

      //! Constructs a SizeDeclarativeReader.
      /*!
        \param source Used to initialize the Reader to read from.
      */
      template<typename SourceReaderForward>
      SizeDeclarativeReader(SourceReaderForward&& source);

      bool IsDataAvailable() const;

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination, std::size_t size);

    private:
      typename OptionalLocalPtr<SourceReaderType>::type m_source;
      std::uint32_t m_sizeRead;
      std::uint32_t m_totalSize;

      void ReadSize();
  };

  template<typename SourceReaderType>
  template<typename SourceReaderForward>
  SizeDeclarativeReader<SourceReaderType>::SizeDeclarativeReader(
      SourceReaderForward&& source)
      : m_source(std::forward<SourceReaderForward>(source)),
        m_sizeRead(0),
        m_totalSize(0) {}

  template<typename SourceReaderType>
  bool SizeDeclarativeReader<SourceReaderType>::IsDataAvailable() const {

    // TODO
    return false;
  }

  template<typename SourceReaderType>
  template<typename BufferType>
  std::size_t SizeDeclarativeReader<SourceReaderType>::Read(
      Out<BufferType> destination) {
    return Read(Store(destination), std::numeric_limits<int>::max());
  }

  template<typename SourceReaderType>
  std::size_t SizeDeclarativeReader<SourceReaderType>::Read(char* destination,
      std::size_t size) {
    if(m_sizeRead == m_totalSize) {
      ReadSize();
    }
    std::size_t offset = 0;
    while(size > 0 && m_sizeRead != m_totalSize) {
      std::size_t nextReadSize = std::min(size,
        static_cast<std::size_t>(m_totalSize - m_sizeRead));
      try {
        std::size_t actualReadSize = m_source->Read(destination + offset,
          nextReadSize);
        offset += actualReadSize;
        m_sizeRead += actualReadSize;
        size -= actualReadSize;
      } catch(const std::exception&) {
        m_sizeRead = 0;
        m_totalSize = 0;
        throw;
      }
    }
    return offset;
  }

  template<typename SourceReaderType>
  template<typename BufferType>
  std::size_t SizeDeclarativeReader<SourceReaderType>::Read(
      Out<BufferType> destination, std::size_t size) {
    if(m_sizeRead == m_totalSize) {
      ReadSize();
    }
    std::size_t initialSize = destination->GetSize();
    destination->Grow(std::min(size,
      static_cast<std::size_t>(m_totalSize - m_sizeRead)));
    return Read(destination->GetMutableData() + initialSize, size);
  }

  template<typename SourceReaderType>
  void SizeDeclarativeReader<SourceReaderType>::ReadSize() {
    char* sizeReadIterator = reinterpret_cast<char*>(&m_totalSize);
    std::ptrdiff_t remainder = (reinterpret_cast<char*>(&m_totalSize) +
      sizeof(std::uint32_t)) - sizeReadIterator;
    while(remainder > 0) {
      try {
        std::size_t readSize = m_source->Read(sizeReadIterator, remainder);
        sizeReadIterator += readSize;
        remainder -= readSize;
      } catch(const std::exception&) {
        m_sizeRead = 0;
        m_totalSize = 0;
        throw;
      }
    }
    m_totalSize = FromLittleEndian(m_totalSize);
    m_sizeRead = 0;
  }
}

  template<typename BufferType, typename SourceReaderType>
  struct ImplementsConcept<IO::SizeDeclarativeReader<SourceReaderType>,
    IO::Reader<BufferType>> : std::true_type {};
}

#endif
