#ifndef BEAM_READER_HPP
#define BEAM_READER_HPP
#include <boost/mpl/if.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Utilities/Concept.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam {
namespace IO {
namespace Details {
  BEAM_DEFINE_HAS_TYPEDEF(ReaderHasBufferType, Buffer);
}

  /*! \struct Reader
      \brief Concept for reading data from a resource.
      \tparam BufferType Specifies the type of Buffer to read into.
   */
  template<typename BufferType>
  struct Reader : Concept<Reader<BufferType>> {
    static_assert(ImplementsConcept<BufferType, IO::Buffer>::value,
      "BufferType must implement the Buffer Concept.");

    //! Specifies a type of Buffer to read into.
    using Buffer = BufferType;

    //! Returns <code>true</code> iff data is available to read without
    //! blocking.
    bool IsDataAvailable() const;

    //! Reads from the resource into a Buffer.
    /*!
      \param destination The Buffer to read to.
      \return The number of bytes read.
    */
    std::size_t Read(Out<Buffer> destination);

    //! Reads from the resource into a specified destination.
    /*!
      \param destination The pointer to read to.
      \param size The maximum size to read.
      \return The number of bytes read.
    */
    std::size_t Read(char* destination, std::size_t size);

    //! Reads a from the resource into a Buffer up to a maximum size.
    /*!
      \param destination The Buffer to read to.
      \param size The maximum number of bytes to read.
      \return The number of bytes read.
    */
    std::size_t Read(Out<Buffer> destination, std::size_t size);
  };

  /*! \struct IsReader
      \brief Tests whether a type satisfies some particular Reader Concept.
      \tparam T The type to test.
   */
  template<typename T, typename Enabled = void>
  struct IsReader : std::false_type {};

  template<typename T>
  struct IsReader<T, typename std::enable_if<
    Details::ReaderHasBufferType<T>::value>::type> : boost::mpl::if_c<
    ImplementsConcept<T, Reader<typename T::Buffer>>::value, std::true_type,
    std::false_type>::type {};

  //! Reads an exact amount of data from a Reader.
  /*!
    \param reader The Reader to read from.
    \param destination The pointer to read into.
    \param size The exact number of bytes to read.
  */
  template<typename Reader>
  void ReadExactSize(Reader& reader, char* destination, std::size_t size) {
    while(size != 0) {
      auto bytesRead = reader.Read(destination, size);
      size -= bytesRead;
      destination += bytesRead;
    }
  }

  //! Reads an exact amount of data from a Reader.
  /*!
    \param reader The Reader to read from.
    \param buffer The Buffer to read into.
    \param size The exact number of bytes to read.
  */
  template<typename Reader, typename Buffer>
  void ReadExactSize(Reader& reader, Out<Buffer> buffer, std::size_t size) {
    buffer->Grow(size);
    auto destination = buffer->GetMutableData() + buffer->GetSize() - size;
    ReadExactSize(reader, destination, size);
  }
}
}

#endif
