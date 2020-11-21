#ifndef BEAM_WRITER_HPP
#define BEAM_WRITER_HPP
#include <type_traits>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Utilities/Concept.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam::IO {
namespace Details {
  BEAM_DEFINE_HAS_TYPEDEF(WriterHasBufferType, Buffer);
}

  /**
   * Interface for writing data to a resource.
   * @param <B> The type of Buffer that gets written.
   */
  template<typename B>
  struct Writer : Concept<Writer<B>> {
    static_assert(ImplementsConcept<B, IO::Buffer>::value,
      "B must implement the Buffer Concept.");

    /** The type of Buffer that gets written. */
    using Buffer = B;

    /**
     * Writes data to the resource.
     * @param data The data to write.
     * @param size The size of the data.
     */
    void Write(const void* data, std::size_t size);

    /**
     * Writes data to the resource.
     * @param data The data to write.
     * @param result The result of the write.
     */
    void Write(const Buffer& data);
  };

  /**
   * Tests whether a type satisfies some particular Writer Concept.
   * @param <T> The type to test.
   */
  template<typename T, typename Enabled = void>
  struct IsWriter : std::false_type {};

  template<typename T>
  struct IsWriter<T, std::enable_if_t<Details::WriterHasBufferType<T>::value>> :
    ImplementsConcept<T, Writer<typename T::Buffer>>::type {};
}

#endif
