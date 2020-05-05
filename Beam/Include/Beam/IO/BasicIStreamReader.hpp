#ifndef BEAM_BASICISTREAMREADER_HPP
#define BEAM_BASICISTREAMREADER_HPP
#include <algorithm>
#include <istream>
#include "Beam/IO/EndOfFileException.hpp"
#include "Beam/IO/Reader.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Pointers/Dereference.hpp"
#include "Beam/Pointers/LocalPtr.hpp"

namespace Beam {
namespace IO {

  /*! \class BasicIStreamReader
      \brief Wraps an std::basic_istream for use with the Reader interface.
      \tparam IStreamType The type of istream to read from.
   */
  template<typename IStreamType>
  class BasicIStreamReader : private boost::noncopyable {
    public:

      //! The default size of a single read operation.
      static const std::size_t DEFAULT_READ_SIZE = 8 * 1024;

      //! The type of istream to read from.
      using IStream = typename TryDereferenceType<IStreamType>::type;

      using Buffer = SharedBuffer;

      //! Constructs a BasicIStreamReader.
      /*!
        \param source Initializes the IStream to read from.
      */
      template<typename IStreamForward>
      BasicIStreamReader(IStreamForward&& source);

      bool IsDataAvailable() const;

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination);

      std::size_t Read(char* destination, std::size_t size);

      template<typename BufferType>
      std::size_t Read(Out<BufferType> destination, std::size_t size);

    private:
      typename OptionalLocalPtr<IStreamType>::type m_source;
  };

  template<typename IStreamType>
  template<typename IStreamForward>
  BasicIStreamReader<IStreamType>::BasicIStreamReader(IStreamForward&& source)
      : m_source(std::forward<IStreamForward>(source)) {}

  template<typename IStreamType>
  bool BasicIStreamReader<IStreamType>::IsDataAvailable() const {
    return m_source->rdbuf()->in_avail() > 0;
  }

  template<typename IStreamType>
  template<typename BufferType>
  std::size_t BasicIStreamReader<IStreamType>::Read(
      Out<BufferType> destination) {
    bool keepReading = true;
    std::size_t result = 0;
    std::size_t previousSize = destination->GetSize();
    while(keepReading) {
      destination->Reserve(result + DEFAULT_READ_SIZE);
      m_source->read(destination->GetMutableData() + result, DEFAULT_READ_SIZE);
      std::streamsize count = m_source->gcount();
      if(count <= 0 && result == 0) {
        BOOST_THROW_EXCEPTION(EndOfFileException());
      }
      result += static_cast<std::size_t>(count);
      keepReading = (count > 0);
    }
    destination->Shrink(destination->GetSize() - (previousSize + result));
    return result;
  }

  template<typename IStreamType>
  std::size_t BasicIStreamReader<IStreamType>::Read(char* destination,
      std::size_t size) {
    m_source->read(destination, size);
    std::streamsize count = m_source->gcount();
    if(count <= 0) {
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    return static_cast<std::size_t>(count);
  }

  template<typename IStreamType>
  template<typename BufferType>
  std::size_t BasicIStreamReader<IStreamType>::Read(Out<BufferType> destination,
      std::size_t size) {
    std::size_t readSize = std::min(DEFAULT_READ_SIZE, size);
    std::size_t previousSize = destination->GetSize();
    destination->Reserve(readSize);
    m_source->read(destination->GetMutableData(), readSize);
    std::streamsize count = m_source->gcount();
    if(count <= 0) {
      destination->Shrink(destination->GetSize() - previousSize);
      BOOST_THROW_EXCEPTION(EndOfFileException());
    }
    std::size_t result = static_cast<std::size_t>(count);
    destination->Shrink(destination->GetSize() - (previousSize + result));
    return result;
  }
}

  template<typename BufferType, typename IStreamType>
  struct ImplementsConcept<IO::BasicIStreamReader<IStreamType>,
    IO::Reader<BufferType>> : std::true_type {};
}

#endif
