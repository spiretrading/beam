#ifndef BEAM_STATIC_BUFFER_HPP
#define BEAM_STATIC_BUFFER_HPP
#include <algorithm>
#include <array>
#include <stdexcept>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"

namespace Beam {

  /**
   * Implements the Buffer Concept using statically allocated memory.
   * @tparam N The maximum size of the Buffer.
   */
  template<std::size_t N>
  class StaticBuffer {
    public:

      /** Constructs an empty StaticBuffer. */
      StaticBuffer() noexcept;

      /**
       * Constructs a StaticBuffer from an existing buffer of raw data.
       * @param data The data to copy.
       * @param size The size of the data to copy.
       */
      StaticBuffer(const void* data, std::size_t size) noexcept;

      /**
       * Constructs a StaticBuffer by copying the contents of another Buffer.
       * @param buffer The buffer to copy from.
       */
      StaticBuffer(const IsConstBuffer auto& buffer);

      StaticBuffer(const StaticBuffer&) = default;

      const char* get_data() const;
      std::size_t get_size() const;
      char* get_mutable_data();
      std::size_t grow(std::size_t size);
      std::size_t shrink(std::size_t size);
      void write(std::size_t index, const void* source, std::size_t size);
      StaticBuffer& operator =(const IsBuffer auto& rhs);
      StaticBuffer& operator =(const StaticBuffer& rhs) = default;

    private:
      std::size_t m_size;
      std::array<char, N> m_data;
  };

  template<std::size_t N>
  StaticBuffer<N>::StaticBuffer() noexcept
    : m_size(0) {}

  template<std::size_t N>
  StaticBuffer<N>::StaticBuffer(const void* data, std::size_t size) noexcept
      : m_size(std::min(size, N)) {
    std::memcpy(m_data.data(), data, size);
  }

  template<std::size_t N>
  StaticBuffer<N>::StaticBuffer(const IsConstBuffer auto& buffer)
    : StaticBuffer(buffer.get_data(), buffer.get_size()) {}

  template<std::size_t N>
  const char* StaticBuffer<N>::get_data() const {
    if(m_size == 0) {
      return nullptr;
    }
    return m_data.data();
  }

  template<std::size_t N>
  std::size_t StaticBuffer<N>::get_size() const {
    return m_size;
  }

  template<std::size_t N>
  char* StaticBuffer<N>::get_mutable_data() {
    if(m_size == 0) {
      return nullptr;
    }
    return m_data.data();
  }

  template<std::size_t N>
  std::size_t StaticBuffer<N>::grow(std::size_t size) {
    auto available = std::min(size, N - m_size);
    m_size += available;
    return available;
  }

  template<std::size_t N>
  std::size_t StaticBuffer<N>::shrink(std::size_t size) {
    auto available = std::min(size, m_size);
    m_size -= available;
    return available;
  }

  template<std::size_t N>
  void StaticBuffer<N>::write(
      std::size_t index, const void* source, std::size_t size) {
    auto end_index = index + size;
    if(end_index > N) {
      boost::throw_with_location(std::out_of_range("Write out of range."));
    }
    if(size > 0) {
      std::memcpy(m_data.data() + index, source, size);
    }
    if(end_index > m_size) {
      m_size = end_index;
    }
  }

  template<std::size_t N>
  StaticBuffer<N>& StaticBuffer<N>::operator =(const IsBuffer auto& rhs) {
    m_size = std::min(rhs.get_size(), N);
    std::memcpy(m_data.data(), rhs.get_data(), m_size);
    return *this;
  }
}

#endif
