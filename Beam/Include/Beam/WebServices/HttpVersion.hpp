#ifndef BEAM_HTTP_VERSION_HPP
#define BEAM_HTTP_VERSION_HPP
#include <ostream>

namespace Beam {

  /** Stores the HTTP version being used. */
  class HttpVersion {
    public:

      /** Returns HTTP version 1.0 */
      static constexpr HttpVersion version_1_0() noexcept;

      /** Returns HTTP version 1.1 */
      static constexpr HttpVersion version_1_1() noexcept;

      /** Constructs a default HttpVersion. */
      constexpr HttpVersion() noexcept;

      /** Returns the major version number. */
      constexpr int get_major() const noexcept;

      /** Returns the minor version number. */
      constexpr int get_minor() const noexcept;

      bool operator ==(const HttpVersion&) const = default;

    private:
      int m_major;
      int m_minor;

      constexpr HttpVersion(int major, int minor) noexcept;
  };

  inline std::ostream& operator <<(
      std::ostream& sink, const HttpVersion& version) {
    if(version == HttpVersion::version_1_1()) {
      sink << "HTTP/1.1";
    } else if(version == HttpVersion::version_1_0()) {
      sink << "HTTP/1.0";
    } else {
      sink << "HTTP/" << version.get_major() << '.' << version.get_minor();
    }
    return sink;
  }

  constexpr HttpVersion HttpVersion::version_1_0() noexcept {
    return HttpVersion(1, 0);
  }

  constexpr HttpVersion HttpVersion::version_1_1() noexcept {
    return HttpVersion(1, 1);
  }

  constexpr HttpVersion::HttpVersion() noexcept
    : HttpVersion(1, 1) {}

  constexpr int HttpVersion::get_major() const noexcept {
    return m_major;
  }

  constexpr int HttpVersion::get_minor() const noexcept {
    return m_minor;
  }

  constexpr HttpVersion::HttpVersion(int major, int minor) noexcept
    : m_major(major),
      m_minor(minor) {}
}

#endif
