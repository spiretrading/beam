#ifndef BEAM_WRITER_HPP
#define BEAM_WRITER_HPP
#include <boost/mpl/if.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Utilities/Concept.hpp"
#include "Beam/Utilities/StaticMemberChecks.hpp"

namespace Beam {
namespace IO {
namespace Details {
  BEAM_DEFINE_HAS_TYPEDEF(WriterHasBufferType, Buffer);
}

  /*! \struct Writer
      \brief Interface for writing data to a resource.
      \tparam BufferType The type of Buffer that gets written.
   */
  template<typename BufferType>
  struct Writer : Concept<Writer<BufferType>> {
    static_assert(ImplementsConcept<BufferType, IO::Buffer>::value,
      "BufferType must implement the Buffer Concept.");

    //! The type of Buffer that gets written.
    using Buffer = BufferType;

    //! Writes data to the resource.
    /*!
      \param data The data to write.
      \param size The size of the data.
    */
    void Write(const void* data, std::size_t size);

    //! Writes data to the resource.
    /*!
      \param data The data to write.
      \param result The result of the write.
    */
    void Write(const Buffer& data);
  };

  /*! \struct IsWriter
      \brief Tests whether a type satisfies some particular Writer Concept.
      \tparam T The type to test.
   */
  template<typename T, typename Enabled = void>
  struct IsWriter : std::false_type {};

  template<typename T>
  struct IsWriter<T, typename std::enable_if<
    Details::WriterHasBufferType<T>::value>::type> : boost::mpl::if_c<
    ImplementsConcept<T, Writer<typename T::Buffer>>::value, std::true_type,
    std::false_type>::type {};
}
}

#endif
