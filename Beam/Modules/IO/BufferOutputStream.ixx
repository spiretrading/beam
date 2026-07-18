module;
#include "Prelude.hpp"

export module Beam:BufferOutputStream;

export namespace Beam {

  /**
   * Allows a Buffer to manipulated as an std::ostream.
   * @tparam B The type of Buffer to manipulate.
   */
  template<IsBuffer B>
  class BaseBufferOutputStream {
    public:

      /** The type of Buffer to manipulate. */
      using Buffer = B;
      using char_type = char;
      using category = boost::iostreams::sink_tag;

      /**
       * Constructs a BufferOutputStream.
       * @param buffer The Buffer to adapt into an std::ostream.
       */
      explicit BaseBufferOutputStream(Ref<Buffer> buffer);

      std::streamsize write(const char_type* s, std::streamsize n);

    private:
      Buffer* m_buffer;
  };

  template<IsBuffer B>
  using BufferOutputStream =
    boost::iostreams::stream<BaseBufferOutputStream<B>>;

  using SharedBufferOutputStream = BufferOutputStream<SharedBuffer>;

  template<IsBuffer B>
  BaseBufferOutputStream<B>::BaseBufferOutputStream(Ref<Buffer> buffer)
    : m_buffer(buffer.get()) {}

  template<IsBuffer B>
  std::streamsize BaseBufferOutputStream<B>::write(
      const char_type* s, std::streamsize n) {
    append(*m_buffer, s, static_cast<std::size_t>(n));
    return n;
  }
}

