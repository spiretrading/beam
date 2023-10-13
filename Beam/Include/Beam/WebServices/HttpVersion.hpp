#ifndef BEAM_HTTP_VERSION_HPP
#define BEAM_HTTP_VERSION_HPP
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /** Stores the HTTP version being used. */
  class HttpVersion {
    public:

      /** Returns HTTP version 1.0 */
      static constexpr HttpVersion Version1_0() noexcept;

      /** Returns HTTP version 1.1 */
      static constexpr HttpVersion Version1_1() noexcept;

      /** Constructs a default HttpVersion. */
      constexpr HttpVersion() noexcept;

      /** Returns the major version number. */
      constexpr int GetMajor() const noexcept;

      /** Returns the minor version number. */
      constexpr int GetMinor() const noexcept;

      bool operator ==(const HttpVersion& version) const = default;

    private:
      int m_major;
      int m_minor;

      constexpr HttpVersion(int major, int minor) noexcept;
  };

  inline std::ostream& operator <<(
      std::ostream& sink, const HttpVersion& version) {
    if(version == HttpVersion::Version1_1()) {
      sink << "HTTP/1.1";
    } else if(version == HttpVersion::Version1_0()) {
      sink << "HTTP/1.0";
    } else {
      sink << "HTTP/" << version.GetMajor() << "." << version.GetMinor();
    }
    return sink;
  }

  constexpr HttpVersion HttpVersion::Version1_0() noexcept {
    return HttpVersion(1, 0);
  }

  constexpr HttpVersion HttpVersion::Version1_1() noexcept {
    return HttpVersion(1, 1);
  }

  constexpr HttpVersion::HttpVersion() noexcept
    : m_major(1),
      m_minor(1) {}

  constexpr int HttpVersion::GetMajor() const noexcept {
    return m_major;
  }

  constexpr int HttpVersion::GetMinor() const noexcept {
    return m_minor;
  }

  constexpr HttpVersion::HttpVersion(int major, int minor) noexcept
    : m_major(major),
      m_minor(minor) {}
}

#endif
