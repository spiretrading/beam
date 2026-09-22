#ifndef BEAM_SHARED_BUFFER_HPP
#define BEAM_SHARED_BUFFER_HPP
#include <algorithm>
#include <bit>
#include <cassert>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"

namespace Beam {

  /** Implements the Buffer Concept using copy-on-write data. */
  class SharedBuffer {
    public:

      /** Constructs an empty SharedBuffer. */
      SharedBuffer() noexcept;

      /**
       * Constructs a SharedBuffer with a pre-allocated initial size.
       * @param initial_size The initial size to pre-allocate.
       */
      explicit SharedBuffer(std::size_t initial_size);

      /**
       * Constructs a SharedBuffer whose contents are a copy of the provided
       * data.
       * @param data The source data to copy from.
       * @param size The size, in bytes, of the source data.
       */
      SharedBuffer(const void* data, std::size_t size);

      /**
       * Constructs a SharedBuffer by copying the contents of another Buffer.
       * @param buffer The buffer to copy from.
       */
      SharedBuffer(const IsConstBuffer auto& buffer);

      SharedBuffer(const SharedBuffer&) = default;
      SharedBuffer(SharedBuffer&& buffer) noexcept;

      /**
       * Returns a copy-on-write view of a byte range.
       * @param offset The first byte of the range.
       * @param size The number of bytes in the range.
       */
      SharedBuffer slice(std::size_t offset, std::size_t size) const;

      const char* get_data() const;
      std::size_t get_size() const;
      char* get_mutable_data();
      std::size_t grow(std::size_t size);
      std::size_t shrink(std::size_t size);
      void write(std::size_t index, const void* source, std::size_t size);
      SharedBuffer& operator =(const IsBuffer auto& rhs);
      SharedBuffer& operator =(const SharedBuffer&) = default;
      SharedBuffer& operator =(SharedBuffer&& rhs) noexcept;

    private:
      std::size_t m_size;
      std::size_t m_capacity;
      std::shared_ptr<char> m_data;

      static std::size_t ceil_power_of_two(std::size_t n);
      void reallocate(std::size_t size);
  };

  inline SharedBuffer::SharedBuffer() noexcept
    : m_size(0),
      m_capacity(0) {}

  inline SharedBuffer::SharedBuffer(std::size_t initial_size)
      : m_size(initial_size),
        m_capacity(ceil_power_of_two(initial_size)) {
    if(m_capacity == 0) {
      return;
    }
    auto data = std::make_shared_for_overwrite<char[]>(m_capacity);
    m_data = std::shared_ptr<char>(std::move(data), data.get());
  }

  inline SharedBuffer::SharedBuffer(const void* data, std::size_t size)
    : m_size(0),
      m_capacity(0) {
    append(*this, data, size);
  }

  inline SharedBuffer::SharedBuffer(const IsConstBuffer auto& buffer)
    : m_size(0),
      m_capacity(0) {
    append(*this, buffer);
  }

  inline SharedBuffer::SharedBuffer(SharedBuffer&& buffer) noexcept
      : m_size(buffer.m_size),
        m_capacity(buffer.m_capacity),
        m_data(std::move(buffer.m_data)) {
    buffer.m_size = 0;
    buffer.m_capacity = 0;
  }

  inline SharedBuffer SharedBuffer::slice(
      std::size_t offset, std::size_t size) const {
    if(offset > m_size || size > m_size - offset) {
      boost::throw_with_location(
        std::out_of_range("Buffer slice out of range."));
    }
    auto slice = SharedBuffer();
    if(size == 0) {
      return slice;
    }
    slice.m_size = size;
    slice.m_capacity = size;
    slice.m_data = std::shared_ptr<char>(m_data, m_data.get() + offset);
    return slice;
  }

  inline const char* SharedBuffer::get_data() const {
    return m_data.get();
  }

  inline std::size_t SharedBuffer::get_size() const {
    return m_size;
  }

  inline char* SharedBuffer::get_mutable_data() {
    if(m_data.use_count() > 1) {
      reallocate(m_capacity);
    }
    return m_data.get();
  }

  inline std::size_t SharedBuffer::grow(std::size_t size) {
    if(m_size + size > m_capacity) {
      reallocate(ceil_power_of_two(m_size + size));
    }
    m_size += size;
    return size;
  }

  inline std::size_t SharedBuffer::shrink(std::size_t size) {
    if(size >= m_size) {
      return std::exchange(m_size, 0);
    }
    m_size -= size;
    return size;
  }

  inline void SharedBuffer::write(
      std::size_t index, const void* source, std::size_t size) {
    assert(index <= m_size);
    if(m_capacity < index + size) {
      reallocate(ceil_power_of_two(index + size));
    } else if(m_data.use_count() > 1) {
      reallocate(m_capacity);
    }
    std::memcpy(m_data.get() + index, source, size);
    m_size = std::max(index + size, m_size);
  }

  inline SharedBuffer& SharedBuffer::operator =(const IsBuffer auto& rhs) {
    m_size = 0;
    append(*this, rhs.get_data(), rhs.get_size());
    return *this;
  }

  inline SharedBuffer& SharedBuffer::operator =(SharedBuffer&& rhs) noexcept {
    m_size = rhs.m_size;
    m_capacity = rhs.m_capacity;
    m_data = std::move(rhs.m_data);
    rhs.m_size = 0;
    rhs.m_capacity = 0;
    return *this;
  }

  inline std::size_t SharedBuffer::ceil_power_of_two(std::size_t n) {
    if(n == 0) {
      return 0;
    }
    if(n > std::bit_floor(std::numeric_limits<std::size_t>::max())) {
      boost::throw_with_location(std::bad_alloc());
    }
    return std::bit_ceil(n);
  }

  inline void SharedBuffer::reallocate(std::size_t size) {
    if(size == 0) {
      m_data.reset();
      m_capacity = 0;
      return;
    }
    auto data = std::make_shared_for_overwrite<char[]>(size);
    std::memcpy(data.get(), m_data.get(), m_size);
    m_data = std::shared_ptr<char>(std::move(data), data.get());
    m_capacity = size;
  }
}

#endif
