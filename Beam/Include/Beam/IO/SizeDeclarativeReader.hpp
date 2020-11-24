#ifndef BEAM_SIZE_DECLARATIVE_READER_HPP
#define BEAM_SIZE_DECLARATIVE_READER_HPP
#include <cstdint>
#include <limits>
#include <type_traits>
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/Endian.hpp"

namespace Beam {
namespace IO {

  /**
   * Reads data whose size is declared at the beginning.
   * @param <R> The type of Reader to read from.
   */
  template<typename R>
  class SizeDeclarativeReader {
    public:

      /** The source to read from. */
      using SourceReader = GetTryDereferenceType<R>;

      /**
       * Constructs a SizeDeclarativeReader.
       * @param source Used to initialize the Reader to read from.
       */
      template<typename RF>
      SizeDeclarativeReader(RF&& source);

      bool IsDataAvailable() const;

      template<typename Buffer>
      std::size_t Read(Out<Buffer> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename Buffer>
      std::size_t Read(Out<Buffer> destination, std::size_t size);

    private:
      GetOptionalLocalPtr<R> m_source;
      std::uint32_t m_sizeRead;
      std::uint32_t m_totalSize;

      SizeDeclarativeReader(const SizeDeclarativeReader&) = delete;
      SizeDeclarativeReader& operator =(const SizeDeclarativeReader&) = delete;
      void ReadSize();
  };

  template<typename R>
  SizeDeclarativeReader(R&&) -> SizeDeclarativeReader<std::decay_t<R>>;

  template<typename R>
  template<typename RF>
  SizeDeclarativeReader<R>::SizeDeclarativeReader(RF&& source)
    : m_source(std::forward<RF>(source)),
      m_sizeRead(0),
      m_totalSize(0) {}

  template<typename R>
  bool SizeDeclarativeReader<R>::IsDataAvailable() const {

    // TODO
    return false;
  }

  template<typename R>
  template<typename Buffer>
  std::size_t SizeDeclarativeReader<R>::Read(Out<Buffer> destination) {
    return Read(Store(destination), std::numeric_limits<std::size_t>::max());
  }

  template<typename R>
  std::size_t SizeDeclarativeReader<R>::Read(char* destination,
      std::size_t size) {
    if(m_sizeRead == m_totalSize) {
      ReadSize();
    }
    auto offset = std::size_t(0);
    while(size > 0 && m_sizeRead != m_totalSize) {
      auto nextReadSize = std::min(size,
        static_cast<std::size_t>(m_totalSize - m_sizeRead));
      try {
        auto actualReadSize = m_source->Read(destination + offset,
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

  template<typename R>
  template<typename Buffer>
  std::size_t SizeDeclarativeReader<R>::Read(
      Out<Buffer> destination, std::size_t size) {
    if(m_sizeRead == m_totalSize) {
      ReadSize();
    }
    auto initialSize = destination->GetSize();
    destination->Grow(std::min(size,
      static_cast<std::size_t>(m_totalSize - m_sizeRead)));
    return Read(destination->GetMutableData() + initialSize, size);
  }

  template<typename R>
  void SizeDeclarativeReader<R>::ReadSize() {
    auto sizeReadIterator = reinterpret_cast<char*>(&m_totalSize);
    auto remainder = (reinterpret_cast<char*>(&m_totalSize) +
      sizeof(std::uint32_t)) - sizeReadIterator;
    while(remainder > 0) {
      try {
        auto readSize = m_source->Read(sizeReadIterator, remainder);
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

  template<typename R>
  struct ImplementsConcept<IO::SizeDeclarativeReader<R>, IO::Reader> :
    std::true_type {};
}

#endif
