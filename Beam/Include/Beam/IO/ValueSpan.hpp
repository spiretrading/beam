#ifndef BEAM_VALUE_SPAN_HPP
#define BEAM_VALUE_SPAN_HPP
#include <stdexcept>
#include <type_traits>
#include <boost/throw_exception.hpp>
#include "Beam/IO/Buffer.hpp"
#include "Beam/Pointers/Ref.hpp"

namespace Beam {

  /**
   * Provides a non-owning Buffer view of an existing object.
   * @tparam T The type of object to provide a view of.
   */
  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  class ValueSpan {
    public:

      /**
       * Constructs a ValueSpan that references an existing object.
       * @param object The object to create a byte-level view over.
       */
      explicit ValueSpan(Ref<T> object) noexcept;

      const char* get_data() const;
      std::size_t get_size() const;
      char* get_mutable_data();
      std::size_t grow(std::size_t size);
      std::size_t shrink(std::size_t size);
      void write(std::size_t index, const void* source, std::size_t size);

    private:
      char* m_data;
      std::size_t m_size;
  };

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  class ValueSpan<const T> {
    public:
      ValueSpan(Ref<const T> object) noexcept;

      const char* get_data() const;
      std::size_t get_size() const;

    private:
      const char* m_data;
      std::size_t m_size;
  };

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  ValueSpan<T>::ValueSpan(Ref<T> object) noexcept
    : m_data(reinterpret_cast<char*>(object.get())),
      m_size(sizeof(T)) {}

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  const char* ValueSpan<T>::get_data() const {
    return m_data;
  }

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  std::size_t ValueSpan<T>::get_size() const {
    return m_size;
  }

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  char* ValueSpan<T>::get_mutable_data() {
    return m_data;
  }

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  std::size_t ValueSpan<T>::grow(std::size_t size) {
    auto old_size = m_size;
    m_size = std::min(m_size + size, sizeof(T));
    return m_size - old_size;
  }

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  std::size_t ValueSpan<T>::shrink(std::size_t size) {
    auto available = std::min(size, m_size);
    m_size -= available;
    return available;
  }

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  void ValueSpan<T>::write(
      std::size_t index, const void* source, std::size_t size) {
    auto end_index = index + size;
    if(end_index > sizeof(T)) {
      boost::throw_with_location(std::out_of_range("Write out of range."));
    }
    auto destination = m_data + index;
    std::memcpy(destination, source, size);
    if(end_index > m_size) {
      m_size = end_index;
    }
  }

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  ValueSpan<const T>::ValueSpan(Ref<const T> object) noexcept
    : m_data(reinterpret_cast<const char*>(object.get())),
      m_size(sizeof(T)) {}

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  const char* ValueSpan<const T>::get_data() const {
    return m_data;
  }

  template<typename T> requires
    std::is_trivially_copyable_v<T> && std::is_standard_layout_v<T>
  std::size_t ValueSpan<const T>::get_size() const {
    return m_size;
  }
}

#endif
