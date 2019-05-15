#ifndef BEAM_STATICBUFFER_HPP
#define BEAM_STATICBUFFER_HPP
#include <array>
#include <cstring>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Utilities/AssertionException.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace IO {

  /*! \class StaticBuffer
      \brief Implements the Buffer Concept using statically allocated memory.
      \tparam N The maximum size of the Buffer.
   */
  template<std::size_t N>
  class StaticBuffer {
    public:

      //! Constructs a StaticBuffer.
      StaticBuffer();

      //! Copies a StaticBuffer.
      /*!
        \param buffer The StaticBuffer to copy.
      */
      StaticBuffer(const StaticBuffer& buffer);

      //! Constructs a StaticBuffer from an existing buffer of raw data.
      /*!
        \param data The data to copy.
        \param size The size of the data to copy.
      */
      StaticBuffer(const void* data, std::size_t size);

      //! Copies a Buffer.
      /*!
        \param buffer The Buffer to copy.
      */
      template<typename Buffer>
      StaticBuffer(const Buffer& buffer);

      StaticBuffer& operator =(const StaticBuffer& rhs);

      template<typename Buffer>
      StaticBuffer& operator =(const Buffer& rhs);

      bool IsEmpty() const;

      void Grow(std::size_t size);

      void Shrink(std::size_t size);

      void ShrinkFront(std::size_t size);

      void Reserve(std::size_t size);

      void Write(std::size_t index, const void* source, std::size_t size);

      template<typename T>
      void Write(std::size_t index, T value);

      template<typename Buffer>
      typename std::enable_if<ImplementsConcept<Buffer,
        IO::Buffer>::value>::type Append(const Buffer& buffer);

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
      std::size_t m_size;
      std::array<char, N> m_data;
      char* m_front;
  };

  template<std::size_t N>
  StaticBuffer<N>::StaticBuffer()
      : m_size(0),
        m_front(&m_data[0]) {}

  template<std::size_t N>
  StaticBuffer<N>::StaticBuffer(const StaticBuffer& buffer)
      : m_size(0),
        m_front(&m_data[0]) {
    Append(data, size);
  }

  template<std::size_t N>
  StaticBuffer<N>::StaticBuffer(const void* data, std::size_t size)
      : m_size(0),
        m_front(&m_data[0]) {
    Append(data, size);
  }

  template<std::size_t N>
  template<typename Buffer>
  StaticBuffer<N>::StaticBuffer(const Buffer& buffer)
      : m_size(0),
        m_front(&m_data[0]) {
    Append(buffer);
  }

  template<std::size_t N>
  StaticBuffer<N>& StaticBuffer<N>::operator =(const StaticBuffer& rhs) {
    if(this == &rhs) {
      return *this;
    }
    Reset();
    Append(rhs);
    return *this;
  }

  template<std::size_t N>
  template<typename Buffer>
  StaticBuffer<N>& StaticBuffer<N>::operator =(const Buffer& rhs) {
    Reset();
    Append(rhs);
    return *this;
  }

  template<std::size_t N>
  bool StaticBuffer<N>::IsEmpty() const {
    return m_size == 0;
  }

  template<std::size_t N>
  void StaticBuffer<N>::Grow(std::size_t size) {
    if(size <= 0) {
      return;
    }
    BEAM_ASSERT_MESSAGE(m_size + size <= N, "Invalid allocation.");
    m_size += size;
  }

  template<std::size_t N>
  void StaticBuffer<N>::Shrink(std::size_t size) {
    if(size == m_size) {
      m_size = 0;
      m_front = &m_data[0];
      return;
    }
    BEAM_ASSERT_MESSAGE(size <= m_size, "Invalid allocation.");
    m_size -= size;
  }

  template<std::size_t N>
  void StaticBuffer<N>::ShrinkFront(std::size_t size) {
    BEAM_ASSERT_MESSAGE(size <= m_size, "Invalid allocation.");
  }

  template<std::size_t N>
  void StaticBuffer<N>::Reserve(std::size_t size) {
    Grow(size - m_size);
  }

  template<std::size_t N>
  void StaticBuffer<N>::Write(std::size_t index, const void* source,
      std::size_t size) {
    BEAM_ASSERT_MESSAGE(m_size + size <= N, "Invalid allocation.");
    std::memcpy(m_front + index, source, size);
    m_size = std::max(index + size, m_size);
  }

  template<std::size_t N>
  template<typename T>
  void StaticBuffer<N>::Write(std::size_t index, T value) {
    Write(index, &value, sizeof(T));
  }

  template<std::size_t N>
  template<typename Buffer>
  typename std::enable_if<ImplementsConcept<Buffer, IO::Buffer>::value>::type
      StaticBuffer<N>::Append(const Buffer& buffer) {
    Append(buffer.GetData(), buffer.GetSize());
  }

  template<std::size_t N>
  void StaticBuffer<N>::Append(const void* data, std::size_t size) {
    if(size == 0) {
      return;
    }
    BEAM_ASSERT_MESSAGE(m_size + size <= N, "Invalid allocation.");
    std::memcpy(m_front + m_size, data, size);
    m_size += size;
  }

  template<std::size_t N>
  template<typename T>
  typename std::enable_if<!ImplementsConcept<T, IO::Buffer>::value>::type
      StaticBuffer<N>::Append(T value) {
    Append(&value, sizeof(T));
  }

  template<std::size_t N>
  void StaticBuffer<N>::Reset() {
    m_size = 0;
    m_front = &m_data[0];
  }

  template<std::size_t N>
  const char* StaticBuffer<N>::GetData() const {
    return m_front;
  }

  template<std::size_t N>
  char* StaticBuffer<N>::GetMutableData() {
    return m_front;
  }

  template<std::size_t N>
  std::size_t StaticBuffer<N>::GetSize() const {
    return m_size;
  }

  template<std::size_t N>
  template<typename T>
  void StaticBuffer<N>::Extract(std::size_t index, Out<T> value) const {
    std::memcpy(reinterpret_cast<char*>(&(*value)), m_front + index, sizeof(T));
  }

  template<std::size_t N>
  template<typename T>
  T StaticBuffer<N>::Extract(std::size_t index) const {
    T value;
    std::memcpy(reinterpret_cast<char*>(&value), m_front + index, sizeof(T));
    return value;
  }
}

  template<std::size_t N>
  struct ImplementsConcept<IO::StaticBuffer<N>, IO::Buffer> : std::true_type {};
}

#endif
