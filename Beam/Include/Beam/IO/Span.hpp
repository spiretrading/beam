#ifndef BEAM_SPAN_HPP
#define BEAM_SPAN_HPP
#include <stdexcept>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"

namespace Beam {

  /** Implements a Buffer on top of a raw pointer. */
  class Span {
    public:

      /** Constructs an empty Span. */
      Span() noexcept;

      /**
       * Constructs a Span from a pointer.
       * @param data The data span.
       * @param size The size of the data.
       */
      Span(void* data, std::size_t size) noexcept;

      const char* get_data() const;
      std::size_t get_size() const;
      char* get_mutable_data();
      std::size_t grow(std::size_t size);
      std::size_t shrink(std::size_t size);
      void write(std::size_t index, const void* source, std::size_t size);

    private:
      char* m_data;
      std::size_t m_size;
      std::size_t m_capacity;
  };

  inline Span::Span() noexcept
    : Span(nullptr, 0) {}

  inline Span::Span(void* data, std::size_t size) noexcept
    : m_data(static_cast<char*>(data)),
      m_size(0),
      m_capacity(size) {}

  inline const char* Span::get_data() const {
    return m_data;
  }

  inline std::size_t Span::get_size() const {
    return m_size;
  }

  inline char* Span::get_mutable_data() {
    return m_data;
  }

  inline std::size_t Span::grow(std::size_t size) {
    auto old_size = m_size;
    m_size = std::min(m_size + size, m_capacity);
    return m_size - old_size;
  }

  inline std::size_t Span::shrink(std::size_t size) {
    if(size >= m_size) {
      return std::exchange(m_size, 0);
    }
    m_size -= size;
    return size;
  }

  inline void Span::write(
      std::size_t index, const void* source, std::size_t size) {
    if(size == 0) {
      return;
    }
    auto end_index = index + size;
    if(end_index > m_capacity) {
      boost::throw_with_location(std::out_of_range("Span write out of range."));
    }
    auto destination = static_cast<char*>(m_data) + index;
    std::memcpy(destination, source, size);
    m_size = std::max(end_index, m_size);
  }
}

#endif
