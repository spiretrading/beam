#ifndef BEAM_BUFFER_SLICE_HPP
#define BEAM_BUFFER_SLICE_HPP
#include <algorithm>
#include "Beam/IO/BufferView.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {
namespace IO {

  /**
   * Provides a Buffer interface over a sub-range of an existing buffer.
   * @param <B> The type of Buffer to slice.
   */
  template<typename B>
  class BufferSlice {
    public:

      /** The type of Buffer to view. */
      using Buffer = B;

      /** Constructs a BufferSlice. */
      BufferSlice(Ref<Buffer> buffer, std::size_t offset);

      bool IsEmpty() const;

      void Grow(std::size_t size);

      void Shrink(std::size_t size);

      void ShrinkFront(std::size_t size);

      void Reserve(std::size_t size);

      void Write(std::size_t index, const void* source, std::size_t size);

      template<typename T>
      void Write(std::size_t index, T value);

      template<typename T>
      std::enable_if_t<IsBufferView<T>> Append(const T& buffer);

      void Append(const void* data, std::size_t size);

      template<typename T>
      std::enable_if_t<!IsBufferView<T>> Append(T value) {
        m_buffer->Append(value);
      }

      void Reset();

      const char* GetData() const;

      char* GetMutableData();

      std::size_t GetSize() const;

      template<typename T>
      void Extract(std::size_t index, Out<T> value) const;

      template<typename T>
      T Extract(std::size_t index) const;

    private:
      Buffer* m_buffer;
      std::size_t m_offset;
  };

  template<typename B>
  BufferSlice<B>::BufferSlice(Ref<Buffer> buffer, std::size_t offset)
    : m_buffer(buffer.Get()),
      m_offset(offset) {}

  template<typename B>
  bool BufferSlice<B>::IsEmpty() const {
    return GetSize() == 0;
  }

  template<typename B>
  void BufferSlice<B>::Grow(std::size_t size) {
    m_buffer->Grow(size);
  }

  template<typename B>
  void BufferSlice<B>::Shrink(std::size_t size) {
    m_buffer->Shrink(std::min(size, GetSize()));
  }

  template<typename B>
  void BufferSlice<B>::ShrinkFront(std::size_t size) {
    m_offset += size;
  }

  template<typename B>
  void BufferSlice<B>::Reserve(std::size_t size) {
    m_buffer->Reserve(size + m_offset);
  }

  template<typename B>
  void BufferSlice<B>::Write(std::size_t index, const void* source,
      std::size_t size) {
    m_buffer->Write(index + m_offset, source, size);
  }

  template<typename B>
  template<typename T>
  void BufferSlice<B>::Write(std::size_t index, T value) {
    Write(index, &value, sizeof(T));
  }

  template<typename B>
  template<typename T>
  std::enable_if_t<IsBufferView<T>> BufferSlice<B>::Append(const T& buffer) {
    m_buffer->Append(buffer);
  }

  template<typename B>
  void BufferSlice<B>::Append(const void* data, std::size_t size) {
    m_buffer->Append(data, size);
  }

  template<typename B>
  void BufferSlice<B>::Reset() {
    if(m_buffer->GetSize() < m_offset) {
      return;
    }
    m_buffer->Shrink(m_buffer->GetSize() - m_offset);
  }

  template<typename B>
  const char* BufferSlice<B>::GetData() const {
    return m_buffer->GetData() + m_offset;
  }

  template<typename B>
  char* BufferSlice<B>::GetMutableData() {
    return m_buffer->GetMutableData() + m_offset;
  }

  template<typename B>
  std::size_t BufferSlice<B>::GetSize() const {
    if(m_offset >= m_buffer->GetSize()) {
      return 0;
    }
    return m_buffer->GetSize() - m_offset;
  }

  template<typename B>
  template<typename T>
  void BufferSlice<B>::Extract(std::size_t index, Out<T> value) const {
    m_buffer->Extract(m_offset + index, Store(value));
  }

  template<typename B>
  template<typename T>
  T BufferSlice<B>::Extract(std::size_t index) const {
    return m_buffer->Extract(index + m_offset);
  }
}

  template<typename B>
  struct ImplementsConcept<IO::BufferSlice<B>, IO::Buffer> :
    std::true_type {};
}

#endif
