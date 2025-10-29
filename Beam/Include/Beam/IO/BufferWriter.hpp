#ifndef BEAM_BUFFER_WRITER_HPP
#define BEAM_BUFFER_WRITER_HPP
#include "Beam/IO/Writer.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {

  /**
   * Appends written data into a Buffer.
   * @tparam B The type of Buffer to write into.
   */
  template<IsBuffer B>
  class BufferWriter {
    public:
      using Buffer = B;

      /**
       * Constructs a BufferWriter that appends into the specified buffer.
       * @param buffer The buffer to append into.
       */
      explicit BufferWriter(Ref<Buffer> buffer) noexcept;

      template<IsConstBuffer T>
      void write(const T& data);

    private:
      Buffer* m_buffer;

      BufferWriter(const BufferWriter&) = delete;
      BufferWriter& operator =(const BufferWriter&) = delete;
  };

  template<IsBuffer B>
  BufferWriter<B>::BufferWriter(Ref<Buffer> buffer) noexcept
    : m_buffer(buffer.get()) {}

  template<IsBuffer B>
  template<IsConstBuffer T>
  void BufferWriter<B>::write(const T& data) {
    append(*m_buffer, data);
  }
}

#endif
