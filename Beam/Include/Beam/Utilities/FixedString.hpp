#ifndef BEAM_FIXED_STRING_HPP
#define BEAM_FIXED_STRING_HPP
#include <algorithm>
#include <array>
#include <compare>
#include <cstring>
#include <ostream>
#include <string_view>

namespace Beam {

  /**
   * String class used to store small strings using a fixed buffer.
   * @tparam N The size of the fixed buffer.
   */
  template<std::size_t N>
  class FixedString {
    public:
      static constexpr auto SIZE = N;

      /** Constructs an empty string. */
      FixedString() noexcept;

      /**
       * Copies a FixedString up to a maximum of N bytes.
       * @param value The FixedString to copy.
       */
      template<std::size_t Q>
      FixedString(const FixedString<Q>& value) noexcept;

      /**
       * Copies a string up to a maximum of N bytes.
       * @param value The value to copy.
       */
      FixedString(const char* value) noexcept;

      /**
       * Copies a string up to a maximum of N bytes.
       * @param value The value to copy.
       */
      FixedString(const std::string& value) noexcept;

      /**
       * Copies a string up to a maximum of N bytes.
       * @param value The value to copy.
       */
      FixedString(std::string_view value) noexcept;

      /** Returns the raw character buffer storing the data. */
      const char* get_data() const;

      /** Returns <code>true</code> iff this FixedString is empty. */
      bool is_empty() const;

      /** Resets to the empty string. */
      void reset();

      /**
       * Assigns from a FixedString up to a maximum of N bytes.
       * @param value The FixedString to assign from.
       */
      template<std::size_t Q>
      FixedString& operator =(const FixedString<Q>& value) noexcept;

      /**
       * Assigns from a string up to a maximum of N bytes.
       * @param value The value to assign from.
       */
      FixedString& operator =(const char* value) noexcept;

      /**
       * Assigns from a string up to a maximum of N bytes.
       * @param value The value to assign from.
       */
      FixedString& operator =(const std::string& value) noexcept;

      /**
       * Assigns from a string up to a maximum of N bytes.
       * @param value The value to assign from.
       */
      FixedString& operator =(std::string_view value) noexcept;

    private:
      std::array<char, N + 1> m_data;
  };

  template<std::size_t N>
  std::ostream& operator <<(std::ostream& sink, const FixedString<N>& value) {
    return sink << value.get_data();
  }

  template<std::size_t L, std::size_t R>
  auto operator <=>(
      const FixedString<L>& lhs, const FixedString<R>& rhs) noexcept {
    auto result = std::strcmp(lhs.get_data(), rhs.get_data());
    if(result < 0) {
      return std::strong_ordering::less;
    } else if(result > 0) {
      return std::strong_ordering::greater;
    }
    return std::strong_ordering::equal;
  }

  template<std::size_t L, std::size_t R>
  bool operator <(
      const FixedString<L>& lhs, const FixedString<R>& rhs) noexcept {
    return (lhs <=> rhs) == std::strong_ordering::less;
  }

  template<std::size_t L, std::size_t R>
  bool operator <=(
      const FixedString<L>& lhs, const FixedString<R>& rhs) noexcept {
    auto order = (lhs <=> rhs);
    return order == std::strong_ordering::less ||
      order == std::strong_ordering::equal;
  }

  template<std::size_t L, std::size_t R>
  bool operator ==(
      const FixedString<L>& lhs, const FixedString<R>& rhs) noexcept {
    return (lhs <=> rhs) == std::strong_ordering::equal;
  }

  template<std::size_t L, std::size_t R>
  bool operator !=(
      const FixedString<L>& lhs, const FixedString<R>& rhs) noexcept {
    return (lhs <=> rhs) != std::strong_ordering::equal;
  }

  template<std::size_t L, std::size_t R>
  bool operator >=(
      const FixedString<L>& lhs, const FixedString<R>& rhs) noexcept {
    auto order = (lhs <=> rhs);
    return order == std::strong_ordering::greater ||
      order == std::strong_ordering::equal;
  }

  template<std::size_t L, std::size_t R>
  bool operator >(
      const FixedString<L>& lhs, const FixedString<R>& rhs) noexcept {
    return (lhs <=> rhs) == std::strong_ordering::greater;
  }

  template<std::size_t N>
  auto operator<=>(const FixedString<N>& lhs, std::string_view rhs) noexcept {
    return std::string_view(lhs.get_data()) <=> rhs;
  }

  template<std::size_t N>
  bool operator <(const FixedString<N>& lhs, std::string_view rhs) noexcept {
    return (lhs <=> rhs) == std::strong_ordering::less;
  }

  template<std::size_t N>
  bool operator <=(const FixedString<N>& lhs, std::string_view rhs) noexcept {
    auto order = (lhs <=> rhs);
    return order == std::strong_ordering::less ||
      order == std::strong_ordering::equal;
  }

  template<std::size_t N>
  bool operator ==(const FixedString<N>& lhs, std::string_view rhs) noexcept {
    return (lhs <=> rhs) == std::strong_ordering::equal;
  }

  template<std::size_t N>
  bool operator !=(const FixedString<N>& lhs, std::string_view rhs) noexcept {
    return (lhs <=> rhs) != std::strong_ordering::equal;
  }

  template<std::size_t N>
  bool operator >=(const FixedString<N>& lhs, std::string_view rhs) noexcept {
    auto order = (lhs <=> rhs);
    return order == std::strong_ordering::greater ||
      order == std::strong_ordering::equal;
  }

  template<std::size_t N>
  bool operator >(const FixedString<N>& lhs, std::string_view rhs) noexcept {
    return (lhs <=> rhs) == std::strong_ordering::greater;
  }

  template<std::size_t N>
  auto operator<=>(std::string_view lhs, const FixedString<N>& rhs) noexcept {
    return lhs <=> std::string_view(rhs.get_data());
  }

  template<std::size_t N>
  bool operator <(std::string_view lhs, const FixedString<N>& rhs) noexcept {
    return (lhs <=> rhs) == std::strong_ordering::less;
  }

  template<std::size_t N>
  bool operator <=(std::string_view lhs, const FixedString<N>& rhs) noexcept {
    auto order = (lhs <=> rhs);
    return order == std::strong_ordering::less ||
      order == std::strong_ordering::equal;
  }

  template<std::size_t N>
  bool operator ==(std::string_view lhs, const FixedString<N>& rhs) noexcept {
    return (lhs <=> rhs) == std::strong_ordering::equal;
  }

  template<std::size_t N>
  bool operator !=(std::string_view lhs, const FixedString<N>& rhs) noexcept {
    return (lhs <=> rhs) != std::strong_ordering::equal;
  }

  template<std::size_t N>
  bool operator >=(std::string_view lhs, const FixedString<N>& rhs) noexcept {
    auto order = (lhs <=> rhs);
    return order == std::strong_ordering::greater ||
      order == std::strong_ordering::equal;
  }

  template<std::size_t N>
  bool operator >(std::string_view lhs, const FixedString<N>& rhs) noexcept {
    return (lhs <=> rhs) == std::strong_ordering::greater;
  }

  template<std::size_t N>
  std::size_t hash_value(const FixedString<N>& value) {
    auto hash = std::size_t(0);
    for(auto p = reinterpret_cast<const unsigned char*>(value.get_data());
        *p != '\0'; ++p) {
      hash = 37 * hash + *p;
    }
    return hash;
  }

  template<std::size_t N>
  FixedString<N>::FixedString() noexcept {
    m_data[0] = '\0';
  }

  template<std::size_t N>
  template<std::size_t Q>
  FixedString<N>::FixedString(const FixedString<Q>& value) noexcept {
    *this = value;
  }

  template<std::size_t N>
  FixedString<N>::FixedString(const char* value) noexcept {
    *this = value;
  }

  template<std::size_t N>
  FixedString<N>::FixedString(const std::string& value) noexcept {
    *this = value;
  }

  template<std::size_t N>
  FixedString<N>::FixedString(std::string_view value) noexcept {
    *this = value;
  }

  template<std::size_t N>
  const char* FixedString<N>::get_data() const {
    return m_data.data();
  }

  template<std::size_t N>
  bool FixedString<N>::is_empty() const {
    return m_data[0] == '\0';
  }

  template<std::size_t N>
  void FixedString<N>::reset() {
    m_data[0] = '\0';
  }

  template<std::size_t N>
  template<std::size_t Q>
  FixedString<N>& FixedString<N>::operator =(
      const FixedString<Q>& value) noexcept {
    auto length = std::min(N, Q);
    std::memcpy(m_data.data(), value.get_data(), length);
    m_data[length] = '\0';
    return *this;
  }

  template<std::size_t N>
  FixedString<N>& FixedString<N>::operator =(const char* value) noexcept {
    return *this = std::string_view(value);
  }

  template<std::size_t N>
  FixedString<N>& FixedString<N>::operator =(
      const std::string& value) noexcept {
    return *this = std::string_view(value);
  }

  template<std::size_t N>
  FixedString<N>& FixedString<N>::operator =(std::string_view value) noexcept {
    auto length = std::min(value.size(), N);
    std::memcpy(m_data.data(), value.data(), length);
    m_data[length] = '\0';
    return *this;
  }
}

namespace std {
  template<std::size_t N>
  struct hash<Beam::FixedString<N>> {
    std::size_t operator ()(const Beam::FixedString<N>& value) const {
      return Beam::hash_value(value);
    }
  };
}

#endif
