#ifndef BEAM_READER_HPP
#define BEAM_READER_HPP
#include <type_traits>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/Pointers/Out.hpp"
#include "Beam/Utilities/Concept.hpp"

namespace Beam::IO {

  /**
   * Concept for reading data from a resource.
   * @param <B> Specifies the type of Buffer to read into.
   */
  struct Reader : Concept<Reader> {

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
    template<typename Buffer>
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
    template<typename Buffer>
    std::size_t Read(Out<Buffer> destination, std::size_t size);
  };

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
