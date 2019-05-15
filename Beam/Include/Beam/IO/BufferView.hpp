#ifndef BEAM_BUFFERVIEW_HPP
#define BEAM_BUFFERVIEW_HPP
#include <algorithm>
#include "Beam/IO/Buffer.hpp"
#include "Beam/IO/IO.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {
namespace IO {

  /*! \class BufferView
      \brief Provides a derived view over an existing Buffer.
      \tparam BufferType The type of Buffer to view.
   */
  template<typename BufferType>
  class BufferView {
    public:

      //! The type of Buffer to view.
      using Buffer = BufferType;

      //! Constructs a BufferView.
      BufferView(Ref<Buffer> buffer, std::size_t offset);

      bool IsEmpty() const;

      void Grow(std::size_t size);

      void Shrink(std::size_t size);

      void ShrinkFront(std::size_t size);

      void Reserve(std::size_t size);

      void Write(std::size_t index, const void* source, std::size_t size);

      template<typename T>
      void Write(std::size_t index, T value);

      template<typename T>
      typename std::enable_if<ImplementsConcept<T, IO::Buffer>::value>::type
        Append(const T& buffer);

      void Append(const void* data, std::size_t size);

      template<typename T>
      typename std::enable_if<!ImplementsConcept<T, IO::Buffer>::value>::type
        Append(T value);

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

  template<typename BufferType>
  BufferView<BufferType>::BufferView(Ref<Buffer> buffer, std::size_t offset)
      : m_buffer(buffer.Get()),
        m_offset(offset) {}

  template<typename BufferType>
  bool BufferView<BufferType>::IsEmpty() const {
    return GetSize() == 0;
  }

  template<typename BufferType>
  void BufferView<BufferType>::Grow(std::size_t size) {
    m_buffer->Grow(size);
  }

  template<typename BufferType>
  void BufferView<BufferType>::Shrink(std::size_t size) {
    m_buffer->Shrink(std::min(size, GetSize()));
  }

  template<typename BufferType>
  void BufferView<BufferType>::ShrinkFront(std::size_t size) {
    m_offset += size;
  }

  template<typename BufferType>
  void BufferView<BufferType>::Reserve(std::size_t size) {
    m_buffer->Reserve(size + m_offset);
  }

  template<typename BufferType>
  void BufferView<BufferType>::Write(std::size_t index, const void* source,
      std::size_t size) {
    m_buffer->Write(index + m_offset, source, size);
  }

  template<typename BufferType>
  template<typename T>
  void BufferView<BufferType>::Write(std::size_t index, T value) {
    Write(index, &value, sizeof(T));
  }

  template<typename BufferType>
  template<typename T>
  typename std::enable_if<ImplementsConcept<T, IO::Buffer>::value>::type
      BufferView<BufferType>::Append(const T& buffer) {
    m_buffer->Append(buffer);
  }

  template<typename BufferType>
  void BufferView<BufferType>::Append(const void* data, std::size_t size) {
    m_buffer->Append(data, size);
  }

  template<typename BufferType>
  template<typename T>
  typename std::enable_if<!ImplementsConcept<T, IO::Buffer>::value>::type
      BufferView<BufferType>::Append(T value) {
    m_buffer->Append(value);
  }

  template<typename BufferType>
  void BufferView<BufferType>::Reset() {
    if(m_buffer->GetSize() < m_offset) {
      return;
    }
    m_buffer->Shrink(m_buffer->GetSize() - m_offset);
  }

  template<typename BufferType>
  const char* BufferView<BufferType>::GetData() const {
    return m_buffer->GetData() + m_offset;
  }

  template<typename BufferType>
  char* BufferView<BufferType>::GetMutableData() {
    return m_buffer->GetMutableData() + m_offset;
  }

  template<typename BufferType>
  std::size_t BufferView<BufferType>::GetSize() const {
    if(m_offset >= m_buffer->GetSize()) {
      return 0;
    }
    return m_buffer->GetSize() - m_offset;
  }

  template<typename BufferType>
  template<typename T>
  void BufferView<BufferType>::Extract(std::size_t index, Out<T> value) const {
    m_buffer->Extract(m_offset + index, Store(value));
  }

  template<typename BufferType>
  template<typename T>
  T BufferView<BufferType>::Extract(std::size_t index) const {
    return m_buffer->Extract(index + m_offset);
  }
}

  template<typename BufferType>
  struct ImplementsConcept<IO::BufferView<BufferType>, IO::Buffer> :
    std::true_type {};
}

#endif
