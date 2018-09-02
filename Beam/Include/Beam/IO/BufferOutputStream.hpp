#ifndef BEAM_BUFFEROUTPUTSTREAM_HPP
#define BEAM_BUFFEROUTPUTSTREAM_HPP
#include <iosfwd>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/stream.hpp>
#include "Beam/IO/IO.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {
namespace IO {

  /*! \class BufferOutputStream
      \brief Allows a Buffer to manipulated as an std::ostream.
      \tparam BufferType The type of Buffer to manipulate.
   */
  template<typename BufferType>
  class BaseBufferOutputStream {
    public:

      //! The type of Buffer to manipulate.
      using Buffer = BufferType;
      using char_type = char;
      using category = boost::iostreams::sink_tag;

      //! Constructs a BufferOutputStream.
      /*!
        \param buffer The Buffer to adapt into an std::ostream.
      */
      BaseBufferOutputStream(Ref<Buffer> buffer);

      std::streamsize write(const char_type* s, std::streamsize n);

    private:
      Buffer* m_buffer;
  };

  template<typename BufferType>
  using BufferOutputStream =
    boost::iostreams::stream<BaseBufferOutputStream<BufferType>>;

  template<typename BufferType>
  BaseBufferOutputStream<BufferType>::BaseBufferOutputStream(Ref<Buffer> buffer)
      : m_buffer(buffer.Get()) {}

  template<typename BufferType>
  std::streamsize BaseBufferOutputStream<BufferType>::write(const char_type* s,
      std::streamsize n) {
    m_buffer->Append(s, static_cast<std::size_t>(n));
    return n;
  }
}
}

#endif
