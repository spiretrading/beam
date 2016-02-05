#ifndef BEAM_HTTPVERSION_HPP
#define BEAM_HTTPVERSION_HPP
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class HttpVersion
      \brief Stores the HTTP version being used.
   */
  class HttpVersion {
    public:

      //! Returns HTTP version 1.0
      static HttpVersion Version1_0();

      //! Returns HTTP version 1.1
      static HttpVersion Version1_1();

      //! Constructs a default HttpVersion.
      constexpr HttpVersion();

      //! Returns the major version number.
      constexpr int GetMajor() const;

      //! Returns the minor version number.
      constexpr int GetMinor() const;

      //! Compares two HttpVersion's for equality.
      /*!
        \param version The HttpVersion to compare to.
        \return <code>true</code> iff the two HttpVersion's are equal.
      */
      constexpr bool operator ==(const HttpVersion& version) const;

      //! Compares two HttpVersion's for inequality.
      /*!
        \param version The HttpVersion to compare to.
        \return <code>true</code> iff the two HttpVersion's are not equal.
      */
      constexpr bool operator !=(const HttpVersion& version) const;

    private:
      int m_major;
      int m_minor;

      constexpr HttpVersion(int major, int minor);
  };

  inline HttpVersion HttpVersion::Version1_0() {
    return HttpVersion{1, 0};
  }

  inline HttpVersion HttpVersion::Version1_1() {
    return HttpVersion{1, 1};
  }

  constexpr HttpVersion::HttpVersion()
      : m_major{1},
        m_minor{1} {}

  constexpr int HttpVersion::GetMajor() const {
    return m_major;
  }

  constexpr int HttpVersion::GetMinor() const {
    return m_minor;
  }

  constexpr bool HttpVersion::operator ==(const HttpVersion& version) const {
    return m_major == version.m_major && m_minor == version.m_minor;
  }

  constexpr bool HttpVersion::operator !=(const HttpVersion& version) const {
    return !(*this == version);
  }

  constexpr HttpVersion::HttpVersion(int major, int minor)
      : m_major{major},
        m_minor{minor} {}
}
}

#endif
