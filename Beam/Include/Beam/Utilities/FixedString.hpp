#ifndef BEAM_FIXED_STRING_HPP
#define BEAM_FIXED_STRING_HPP
#include <cstring>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /** String class used to store small strings using a fixed buffer.
      \tparam N The size of the fixed buffer.
   */
  template<std::size_t N>
  class FixedString {
    public:
      static constexpr auto SIZE = N;

      //! Constructs an empty string.
      FixedString();

      //! Copies a FixedString up to a maximum of N bytes.
      /*!
        \param fixedString The FixedString to copy.
      */
      template<std::size_t Q>
      FixedString(const FixedString<Q>& fixedString);

      //! Copies a C string up to a maximum of N bytes.
      /*!
        \param data The data to copy.
      */
      FixedString(const char* data);

      //! Copies a raw buffer up to a maximum of N bytes.
      /*!
        \param data The data to copy.
        \param size The size of the data to copy.
      */
      FixedString(const char* data, std::size_t size);

      //! Copies a string up to a maximum of N bytes.
      /*!
        \param data The data to copy.
      */
      FixedString(const std::string& data);

      //! Assigns from a FixedString up to a maximum of N bytes.
      /*!
        \param fixedString The FixedString to assign from.
      */
      template<std::size_t Q>
      FixedString& operator =(const FixedString<Q>& fixedString);

      //! Assigns from a C string up to a maximum of N bytes.
      /*!
        \param data The data to assign from.
      */
      FixedString& operator =(const char* data);

      //! Assigns from a string up to a maximum of N bytes.
      /*!
        \param data The data to assign from.
      */
      FixedString& operator =(const std::string& data);

      //! Returns the raw character buffer storing the data.
      const char* GetData() const;

      //! Returns <code>true</code> iff this FixedString is empty.
      bool IsEmpty() const;

      //! Resets to the empty string.
      void Reset();

    private:
      char m_data[N + 1];
  };

  template<std::size_t N>
  std::ostream& operator <<(std::ostream& sink, const FixedString<N>& value) {
    return (sink << value.GetData());
  }

  template<std::size_t L, std::size_t R>
  bool operator ==(const FixedString<L>& lhs, const FixedString<R>& rhs) {
    return std::strcmp(lhs.GetData(), rhs.GetData()) == 0;
  }

  template<std::size_t N>
  bool operator ==(const FixedString<N>& lhs, const std::string& rhs) {
    return lhs.GetData() == rhs;
  }

  template<std::size_t N>
  bool operator ==(const std::string& lhs, const FixedString<N>& rhs) {
    return lhs == rhs.GetData();
  }

  template<std::size_t N>
  bool operator ==(const FixedString<N>& lhs, const char* rhs) {
    return std::strcmp(lhs.GetData(), rhs) == 0;
  }

  template<std::size_t N>
  bool operator ==(const char* lhs, const FixedString<N>& rhs) {
    return std::strcmp(lhs, rhs.GetData()) == 0;
  }

  template<std::size_t L, std::size_t R>
  bool operator !=(const FixedString<L>& lhs, const FixedString<R>& rhs) {
    return !(lhs == rhs);
  }

  template<std::size_t N>
  bool operator !=(const FixedString<N>& lhs, const std::string& rhs) {
    return !(lhs == rhs);
  }

  template<std::size_t N>
  bool operator !=(const std::string& lhs, const FixedString<N>& rhs) {
    return !(lhs == rhs);
  }

  template<std::size_t N>
  bool operator !=(const FixedString<N>& lhs, const char* rhs) {
    return !(lhs == rhs);
  }

  template<std::size_t N>
  bool operator !=(const char* lhs, const FixedString<N>& rhs) {
    return !(lhs == rhs);
  }

  template<std::size_t L, std::size_t R>
  bool operator <(const FixedString<L>& lhs, const FixedString<R>& rhs) {
    return std::strcmp(lhs.GetData(), rhs.GetData()) < 0;
  }

  template<std::size_t N>
  bool operator <(const FixedString<N>& lhs, const std::string& rhs) {
    return lhs.GetData() < rhs;
  }

  template<std::size_t N>
  bool operator <(const std::string& lhs, const FixedString<N>& rhs) {
    return lhs < rhs.GetData();
  }

  template<std::size_t N>
  bool operator <(const FixedString<N>& lhs, const char* rhs) {
    return std::strcmp(lhs.GetData(), rhs) < 0;
  }

  template<std::size_t N>
  bool operator <(const char* lhs, const FixedString<N>& rhs) {
    return std::strcmp(lhs, rhs.GetData()) < 0;
  }

  template<std::size_t L, std::size_t R>
  bool operator <=(const FixedString<L>& lhs, const FixedString<R>& rhs) {
    return std::strcmp(lhs.GetData(), rhs.GetData()) <= 0;
  }

  template<std::size_t N>
  bool operator <=(const FixedString<N>& lhs, const std::string& rhs) {
    return lhs.GetData() <= rhs;
  }

  template<std::size_t N>
  bool operator <=(const std::string& lhs, const FixedString<N>& rhs) {
    return lhs <= rhs.GetData();
  }

  template<std::size_t N>
  bool operator <=(const FixedString<N>& lhs, const char* rhs) {
    return std::strcmp(lhs.GetData(), rhs) <= 0;
  }

  template<std::size_t N>
  bool operator <=(const char* lhs, const FixedString<N>& rhs) {
    return std::strcmp(lhs, rhs.GetData()) <= 0;
  }

  template<std::size_t L, std::size_t R>
  bool operator >=(const FixedString<L>& lhs, const FixedString<R>& rhs) {
    return std::strcmp(lhs.GetData(), rhs.GetData()) >= 0;
  }

  template<std::size_t N>
  bool operator >=(const FixedString<N>& lhs, const std::string& rhs) {
    return lhs.GetData() >= rhs;
  }

  template<std::size_t N>
  bool operator >=(const std::string& lhs, const FixedString<N>& rhs) {
    return lhs >= rhs.GetData();
  }

  template<std::size_t N>
  bool operator >=(const FixedString<N>& lhs, const char* rhs) {
    return std::strcmp(lhs.GetData(), rhs) >= 0;
  }

  template<std::size_t N>
  bool operator >=(const char* lhs, const FixedString<N>& rhs) {
    return std::strcmp(lhs, rhs.GetData()) >= 0;
  }

  template<std::size_t L, std::size_t R>
  bool operator >(const FixedString<L>& lhs, const FixedString<R>& rhs) {
    return std::strcmp(lhs.GetData(), rhs.GetData()) > 0;
  }

  template<std::size_t N>
  bool operator >(const FixedString<N>& lhs, const std::string& rhs) {
    return lhs.GetData() > rhs;
  }

  template<std::size_t N>
  bool operator >(const std::string& lhs, const FixedString<N>& rhs) {
    return lhs > rhs.GetData();
  }

  template<std::size_t N>
  bool operator >(const FixedString<N>& lhs, const char* rhs) {
    return std::strcmp(lhs.GetData(), rhs) > 0;
  }

  template<std::size_t N>
  bool operator >(const char* lhs, const FixedString<N>& rhs) {
    return std::strcmp(lhs, rhs.GetData()) > 0;
  }

  template<std::size_t N>
  std::size_t hash_value(const FixedString<N>& value) {
    std::size_t h = 0;
    for(const unsigned char* p = reinterpret_cast<const unsigned char*>(
        value.GetData()); *p != '\0'; ++p) {
      h = 37 * h + *p;
    }
    return h;
  }

  template<std::size_t N>
  FixedString<N>::FixedString() {
    m_data[0] = '\0';
  }

  template<std::size_t N>
  template<std::size_t Q>
  FixedString<N>::FixedString(const FixedString<Q>& fixedString) {
    *this = fixedString;
  }

  template<std::size_t N>
  FixedString<N>::FixedString(const char* data) {
    *this = data;
  }

  template<std::size_t N>
  FixedString<N>::FixedString(const char* data, std::size_t size) {
    std::strncpy(m_data, data, N);
    m_data[N] = '\0';
  }

  template<std::size_t N>
  FixedString<N>::FixedString(const std::string& data) {
    *this = data;
  }

  template<std::size_t N>
  template<std::size_t Q>
  FixedString<N>& FixedString<N>::operator =(
      const FixedString<Q>& fixedString) {
    std::memcpy(m_data, fixedString.GetData(), std::min(N + 1, Q + 1));
    if(N < Q) {
      m_data[N] = '\0';
    } else if(N > Q) {
      m_data[Q] = '\0';
    }
    return *this;
  }

  template<std::size_t N>
  FixedString<N>& FixedString<N>::operator =(const char* data) {
    std::strncpy(m_data, data, N);
    m_data[N] = '\0';
    return *this;
  }

  template<std::size_t N>
  FixedString<N>& FixedString<N>::operator =(const std::string& data) {
    std::strncpy(m_data, data.c_str(), N);
    m_data[N] = '\0';
    return *this;
  }

  template<std::size_t N>
  const char* FixedString<N>::GetData() const {
    return m_data;
  }

  template<std::size_t N>
  bool FixedString<N>::IsEmpty() const {
    return m_data[0] == '\0';
  }

  template<std::size_t N>
  void FixedString<N>::Reset() {
    m_data[0] = '\0';
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
