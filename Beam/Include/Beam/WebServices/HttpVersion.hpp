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

      //! Returns the major version number.
      constexpr int GetMajor() const;

      //! Returns the minor version number.
      constexpr int GetMinor() const;

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

  constexpr int HttpVersion::GetMajor() const {
    return m_major;
  }

  constexpr int HttpVersion::GetMinor() const {
    return m_minor;
  }

  constexpr HttpVersion::HttpVersion(int major, int minor)
      : m_major{major},
        m_minor{minor} {}
}
}

#endif
