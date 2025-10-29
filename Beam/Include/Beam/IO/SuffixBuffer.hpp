#ifndef BEAM_SUFFIX_BUFFER_HPP
#define BEAM_SUFFIX_BUFFER_HPP
#include <stdexcept>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {
  template<typename B>
  class SuffixBuffer;

  /** Provides a Buffer view over the suffix of an existing buffer. */
  template<typename B> requires(IsBuffer<B> && !std::is_const_v<B>)
  class SuffixBuffer<B> {
    public:

      /** The type of Buffer being viewed. */
      using Buffer = B;

      /*
       * Constructs a SuffixBuffer.
       * @param buffer The buffer to create the suffix from.
       * @param offset The start offset within <i>buffer</i>.
       */
      SuffixBuffer(Ref<Buffer> buffer, std::size_t offset);

      const char* get_data() const;
      std::size_t get_size() const;
      char* get_mutable_data();
      std::size_t grow(std::size_t size);
      std::size_t shrink(std::size_t size);
      void write(std::size_t index, const void* source, std::size_t size);

    private:
      Buffer* m_buffer;
      std::size_t m_offset;
  };

  template<typename B> requires IsConstBuffer<B>
  class SuffixBuffer<const B> {
    public:
      using Buffer = const B;
      SuffixBuffer(Ref<Buffer> buffer, std::size_t offset);
      const char* get_data() const;
      std::size_t get_size() const;

    private:
      Buffer* m_buffer;
      std::size_t m_offset;
  };

  template<typename B> requires IsConstBuffer<B>
  SuffixBuffer(Ref<const B>, std::size_t) -> SuffixBuffer<const B>;

  template<typename B> requires(IsBuffer<B> && !std::is_const_v<B>)
  SuffixBuffer(Ref<B>, std::size_t) -> SuffixBuffer<B>;

  /**
   * Creates a SuffixBuffer.
   * @param buffer The buffer to create the suffix from.
   * @param offset The start offset within <i>buffer</i>.
   * @return The constructed SuffixBuffer.
   */
  template<IsConstBuffer B>
  auto suffix(Ref<B> buffer, std::size_t offset) {
    return SuffixBuffer(Ref(buffer), offset);
  }

  template<typename B> requires(IsBuffer<B> && !std::is_const_v<B>)
  SuffixBuffer<B>::SuffixBuffer(Ref<Buffer> buffer, std::size_t offset)
    : m_buffer(buffer.get()),
      m_offset(offset) {}

  template<typename B> requires(IsBuffer<B> && !std::is_const_v<B>)
  const char* SuffixBuffer<B>::get_data() const {
    if(m_offset >= m_buffer->get_size()) {
      return nullptr;
    }
    return m_buffer->get_data() + m_offset;
  }

  template<typename B> requires(IsBuffer<B> && !std::is_const_v<B>)
  std::size_t SuffixBuffer<B>::get_size() const {
    auto total = m_buffer->get_size();
    if(m_offset >= total) {
      return 0;
    }
    return total - m_offset;
  }

  template<typename B> requires(IsBuffer<B> && !std::is_const_v<B>)
  char* SuffixBuffer<B>::get_mutable_data() {
    if(m_offset >= m_buffer->get_size()) {
      return nullptr;
    }
    return m_buffer->get_mutable_data() + m_offset;
  }

  template<typename B> requires(IsBuffer<B> && !std::is_const_v<B>)
  std::size_t SuffixBuffer<B>::grow(std::size_t size) {
    return m_buffer->grow(size);
  }

  template<typename B> requires(IsBuffer<B> && !std::is_const_v<B>)
  std::size_t SuffixBuffer<B>::shrink(std::size_t size) {
    return m_buffer->shrink(std::min(size, get_size()));
  }

  template<typename B> requires(IsBuffer<B> && !std::is_const_v<B>)
  void SuffixBuffer<B>::write(
      std::size_t index, const void* source, std::size_t size) {
    m_buffer->write(m_offset + index, source, size);
  }

  template<typename B> requires IsConstBuffer<B>
  SuffixBuffer<const B>::SuffixBuffer(Ref<Buffer> buffer, std::size_t offset)
    : m_buffer(buffer.get()),
      m_offset(offset) {}

  template<typename B> requires IsConstBuffer<B>
  const char* SuffixBuffer<const B>::get_data() const {
    if(m_offset >= m_buffer->get_size()) {
      return nullptr;
    }
    return m_buffer->get_data() + m_offset;
  }

  template<typename B> requires IsConstBuffer<B>
  std::size_t SuffixBuffer<const B>::get_size() const {
    auto total = m_buffer->get_size();
    if(m_offset >= total) {
      return 0;
    }
    return total - m_offset;
  }
}

#endif
