#ifndef BEAM_SHAREDBUFFER_HPP
#define BEAM_SHAREDBUFFER_HPP
#include <cstring>
#include <new>
#include <boost/shared_array.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"

namespace Beam {
namespace IO {
namespace Details {
  inline std::size_t FindNextPowerOfTwo(std::size_t current) {
    std::size_t nextPowerOfTwo = 1;
    std::size_t lastPowerOfTwo = nextPowerOfTwo;
    while(nextPowerOfTwo < current) {
      nextPowerOfTwo *= 2;
      if(nextPowerOfTwo < lastPowerOfTwo) {
        BOOST_THROW_EXCEPTION(std::bad_alloc());
      }
      lastPowerOfTwo = nextPowerOfTwo;
    }
    return nextPowerOfTwo;
  }
}

  /*! \class SharedBuffer
      \brief Implements the Buffer Concept using copy-on-write data.
   */
  class SharedBuffer {
    public:

      //! Constructs a SharedBuffer.
      SharedBuffer();

      //! Constructs a SharedBuffer with a pre-allocated initial size.
      /*!
        \param initialSize The initial size to pre-allocate.
      */
      SharedBuffer(std::size_t initialSize);

      SharedBuffer(const void* data, std::size_t size);

      SharedBuffer(const SharedBuffer& buffer);

      template<typename BufferType>
      SharedBuffer(const BufferType& buffer, typename std::enable_if<
        ImplementsConcept<BufferType, Buffer>::value>::type* = 0);

      SharedBuffer(SharedBuffer&& buffer);

      SharedBuffer& operator =(const SharedBuffer& rhs);

      template<typename Buffer>
      SharedBuffer& operator =(const Buffer& rhs);

      SharedBuffer& operator =(SharedBuffer&& rhs);

      bool IsEmpty() const;

      void Grow(std::size_t size);

      void Shrink(std::size_t size);

      void ShrinkFront(std::size_t size);

      void Reserve(std::size_t size);

      void Write(std::size_t index, const void* source, std::size_t size);

      template<typename T>
      void Write(std::size_t index, T value);

      void Append(const SharedBuffer& buffer);

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
      std::size_t m_availableSize;
      boost::shared_array<char> m_data;
      char* m_front;

      void Reallocate();
  };

  inline SharedBuffer::SharedBuffer()
      : m_size(0),
        m_availableSize(0),
        m_front(nullptr) {}

  inline SharedBuffer::SharedBuffer(std::size_t initialSize)
      : m_size(initialSize),
        m_availableSize(Details::FindNextPowerOfTwo(initialSize)),
        m_data(new char[m_availableSize]),
        m_front(m_data.get()) {}

  inline SharedBuffer::SharedBuffer(const void* data, std::size_t size)
      : m_size(0),
        m_availableSize(0),
        m_front(nullptr) {
    Append(data, size);
  }

  inline SharedBuffer::SharedBuffer(const SharedBuffer& buffer)
      : m_size(buffer.m_size),
        m_availableSize(buffer.m_availableSize),
        m_data(buffer.m_data),
        m_front(buffer.m_front) {}

  template<typename BufferType>
  SharedBuffer::SharedBuffer(const BufferType& buffer, typename std::enable_if<
      ImplementsConcept<BufferType, Buffer>::value>::type*)
      : m_size(0),
        m_availableSize(0),
        m_front(nullptr) {
    Append(buffer);
  }

  inline SharedBuffer::SharedBuffer(SharedBuffer&& buffer)
      : m_size(std::move(buffer.m_size)),
        m_availableSize(std::move(buffer.m_availableSize)),
        m_data(std::move(buffer.m_data)),
        m_front(std::move(buffer.m_front)) {}

  inline SharedBuffer& SharedBuffer::operator =(const SharedBuffer& rhs) {
    m_size = rhs.m_size;
    m_availableSize = rhs.m_availableSize;
    m_data = rhs.m_data;
    m_front = rhs.m_front;
    return *this;
  }

  template<typename Buffer>
  SharedBuffer& SharedBuffer::operator =(const Buffer& rhs) {
    Reset();
    Append(rhs);
    return *this;
  }

  inline SharedBuffer& SharedBuffer::operator =(SharedBuffer&& rhs) {
    m_size = std::move(rhs.m_size);
    m_availableSize = std::move(rhs.m_availableSize);
    m_data = std::move(rhs.m_data);
    m_front = std::move(rhs.m_front);
    return *this;
  }

  inline bool SharedBuffer::IsEmpty() const {
    return m_size == 0;
  }

  inline void SharedBuffer::Grow(std::size_t size) {
    if(m_size + size > m_availableSize) {
      m_availableSize = Details::FindNextPowerOfTwo(m_size + size);
      Reallocate();
    }
    m_size += size;
  }

  inline void SharedBuffer::Shrink(std::size_t size) {
    if(size >= m_size) {
      m_size = 0;
      m_front = m_data.get();
      return;
    }
    m_size -= size;
  }

  inline void SharedBuffer::ShrinkFront(std::size_t size) {
    assert(size >= 0);
    auto data = boost::shared_array<char>{new char[m_availableSize]};
    std::memcpy(data.get(), m_data.get() + size, m_size - size);
    data.swap(m_data);
    m_size -= size;
    m_front = m_data.get();
  }

  inline void SharedBuffer::Reserve(std::size_t size) {
    Grow(size - m_size);
  }

  inline void SharedBuffer::Write(std::size_t index, const void* source,
      std::size_t size) {
    assert(index <= m_size);
    if(m_availableSize < index + size) {
      m_availableSize = Details::FindNextPowerOfTwo(index + size);
      Reallocate();
    } else if(!m_data.unique()) {
      Reallocate();
    }
    std::memcpy(m_front + index, source, size);
    m_size = std::max(index + size, m_size);
  }

  template<typename T>
  void SharedBuffer::Write(std::size_t index, T value) {
    Write(index, &value, sizeof(T));
  }

  inline void SharedBuffer::Append(const SharedBuffer& buffer) {
    if(m_size == 0) {
      *this = buffer;
      return;
    }
    Append(buffer.GetData(), buffer.GetSize());
  }

  template<typename Buffer>
  typename std::enable_if<ImplementsConcept<Buffer, IO::Buffer>::value>::type
      SharedBuffer::Append(const Buffer& buffer) {
    Append(buffer.GetData(), buffer.GetSize());
  }

  inline void SharedBuffer::Append(const void* data, std::size_t size) {
    if(m_availableSize < m_size + size) {
      m_availableSize = Details::FindNextPowerOfTwo(m_size + size);
      Reallocate();
    } else if(!m_data.unique()) {
      Reallocate();
    }
    std::memcpy(m_front + m_size, data, size);
    m_size += size;
  }

  template<typename T>
  typename std::enable_if<!ImplementsConcept<T, IO::Buffer>::value>::type
      SharedBuffer::Append(T value) {
    Append(&value, sizeof(T));
  }

  inline void SharedBuffer::Reset() {
    m_size = 0;
    m_front = m_data.get();
  }

  inline const char* SharedBuffer::GetData() const {
    return m_front;
  }

  inline char* SharedBuffer::GetMutableData() {
    if(!m_data.unique()) {
      Reallocate();
    }
    return m_front;
  }

  inline std::size_t SharedBuffer::GetSize() const {
    return m_size;
  }

  template<typename T>
  void SharedBuffer::Extract(std::size_t index, Out<T> value) const {
    std::memcpy(reinterpret_cast<char*>(&(*value)), m_front + index, sizeof(T));
  }

  template<typename T>
  T SharedBuffer::Extract(std::size_t index) const {
    T value;
    std::memcpy(reinterpret_cast<char*>(&value), m_front + index, sizeof(T));
    return value;
  }

  inline void SharedBuffer::Reallocate() {
    auto oldData = std::move(m_data);
    m_data.reset(new char[m_availableSize]);
    std::memcpy(m_data.get(), oldData.get(), m_size);
    m_front = m_data.get() + (m_front - oldData.get());
  }
}

  template<>
  struct ImplementsConcept<IO::SharedBuffer, IO::Buffer> : std::true_type {};
}

#endif
