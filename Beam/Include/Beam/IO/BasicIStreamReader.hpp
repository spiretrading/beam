#ifndef BEAM_BASIC_ISTREAM_READER_HPP
#define BEAM_BASIC_ISTREAM_READER_HPP
#include <algorithm>
#include <istream>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /**
   * Wraps an std::basic_istream for use with the Reader interface.
   * @param <S> The type of istream to read from.
   */
  template<typename S>
  class BasicIStreamReader {
    public:

      /** The default size of a single read operation. */
      static constexpr auto DEFAULT_READ_SIZE = std::size_t(8 * 1024);

      /** The type of istream to read from. */
      using IStream = GetTryDereferenceType<S>;

      using Buffer = SharedBuffer;

      /**
       * Constructs a BasicIStreamReader.
       * @param source Initializes the IStream to read from.
       */
      template<typename SF>
      BasicIStreamReader(SF&& source);

      bool IsDataAvailable() const;

      template<typename B>
      std::size_t Read(Out<B> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename B>
      std::size_t Read(Out<B> destination, std::size_t size);

    private:
      GetOptionalLocalPtr<S> m_source;

      BasicIStreamReader(const BasicIStreamReader&) = delete;
      BasicIStreamReader& operator =(const BasicIStreamReader&) = delete;
  };

  template<typename S>
  template<typename SF>
  BasicIStreamReader<S>::BasicIStreamReader(SF&& source)
    : m_source(std::forward<SF>(source)) {}

  template<typename S>
  bool BasicIStreamReader<S>::IsDataAvailable() const {
    return m_source->rdbuf()->in_avail() > 0;
  }

  template<typename S>
  template<typename B>
  std::size_t BasicIStreamReader<S>::Read(Out<B> destination) {
    auto keepReading = true;
    auto result = std::size_t(0);
    auto previousSize = destination->GetSize();
    while(keepReading) {
      destination->Reserve(result + DEFAULT_READ_SIZE);
      m_source->read(destination->GetMutableData() + result, DEFAULT_READ_SIZE);
      auto count = m_source->gcount();
      if(count <= 0 && result == 0) {
        BOOST_THROW_EXCEPTION(EndOfFileException());
      }
      result += static_cast<std::size_t>(count);
      keepReading = (count > 0);
    }
    destination->Shrink(destination->GetSize() - (previousSize + result));
    return result;
  }

  template<typename S>
  std::size_t BasicIStreamReader<S>::Read(char* destination, std::size_t size) {
    m_source->read(destination, size);
    auto count = m_source->gcount();
    if(count <= 0) {
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    return static_cast<std::size_t>(count);
  }

  template<typename S>
  template<typename B>
  std::size_t BasicIStreamReader<S>::Read(Out<B> destination,
      std::size_t size) {
    auto readSize = std::min(DEFAULT_READ_SIZE, size);
    auto previousSize = destination->GetSize();
    destination->Reserve(readSize);
    m_source->read(destination->GetMutableData(), readSize);
    auto count = m_source->gcount();
    if(count <= 0) {
      destination->Shrink(destination->GetSize() - previousSize);
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    auto result = static_cast<std::size_t>(count);
    destination->Shrink(destination->GetSize() - (previousSize + result));
    return result;
  }
}

  template<typename BufferType, typename S>
  struct ImplementsConcept<IO::BasicIStreamReader<S>, IO::Reader<BufferType>> :
    std::true_type {};
}

#endif
