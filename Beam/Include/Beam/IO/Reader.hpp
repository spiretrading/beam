#ifndef BEAM_READER_HPP
#define BEAM_READER_HPP
#include <type_traits>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Utilities/Concept.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam::IO {
namespace Details {
  BEAM_DEFINE_HAS_TYPEDEF(ReaderHasBufferType, Buffer);
}

  /**
   * Concept for reading data from a resource.
   * @param <B> Specifies the type of Buffer to read into.
   */
  template<typename B>
  struct Reader : Concept<Reader<B>> {
    static_assert(ImplementsConcept<B, IO::Buffer>::value,
      "B must implement the Buffer Concept.");

    /** Specifies a type of Buffer to read into. */
    using Buffer = B;

    /**
     * Returns <code>true</code> iff data is available to read without
     * blocking.
     */
    bool IsDataAvailable() const;

    /**
     * Reads from the resource into a Buffer.
     * @param destination The Buffer to read to.
     * @return The number of bytes read.
     */
    std::size_t Read(Out<Buffer> destination);

    /**
     * Reads from the resource into a specified destination.
     * @param destination The pointer to read to.
     * @param size The maximum size to read.
     * @return The number of bytes read.
     */
    std::size_t Read(char* destination, std::size_t size);

    /**
     * Reads a from the resource into a Buffer up to a maximum size.
     * @param destination The Buffer to read to.
     * @param size The maximum number of bytes to read.
     * @return The number of bytes read.
     */
    std::size_t Read(Out<Buffer> destination, std::size_t size);
  };

  /**
   * Tests whether a type satisfies some particular Reader Concept.
   * @param <T> The type to test.
   */
  template<typename T, typename Enabled = void>
  struct IsReader : std::false_type {};

  template<typename T>
  struct IsReader<T, std::enable_if_t<Details::ReaderHasBufferType<T>::value>> :
    ImplementsConcept<T, Reader<typename T::Buffer>>::type {};

  /**
   * Reads an exact amount of data from a Reader.
   * @param reader The Reader to read from.
   * @param destination The pointer to read into.
   * @param size The exact number of bytes to read.
   */
  template<typename Reader>
  void ReadExactSize(Reader& reader, char* destination, std::size_t size) {
    while(size != 0) {
      auto bytesRead = reader.Read(destination, size);
      size -= bytesRead;
      destination += bytesRead;
    }
  }

  /**
   * Reads an exact amount of data from a Reader.
   * @param reader The Reader to read from.
   * @param buffer The Buffer to read into.
   * @param size The exact number of bytes to read.
   */
  template<typename Reader, typename Buffer>
  void ReadExactSize(Reader& reader, Out<Buffer> buffer, std::size_t size) {
    buffer->Grow(size);
    auto destination = buffer->GetMutableData() + buffer->GetSize() - size;
    ReadExactSize(reader, destination, size);
  }
}

#endif
