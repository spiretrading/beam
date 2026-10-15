#ifndef BEAM_HTTP_HEADER_HPP
#define BEAM_HTTP_HEADER_HPP
#include <format>
#include <ostream>
#include <string>
#include <string_view>

namespace Beam {

  /** Stores an HTTP header. */
  class HttpHeader {
    public:

      /**
       * Constructs an HttpHeader.
       * @param name The header's name.
       * @param value The header's value.
       */
      HttpHeader(std::string name, std::string value) noexcept;

      /** Returns the header's name. */
      const std::string& get_name() const;

      /** Returns the header's value. */
      const std::string& get_value() const;

    private:
      std::string m_name;
      std::string m_value;
  };

  inline HttpHeader::HttpHeader(std::string name, std::string value) noexcept
    : m_name(std::move(name)),
      m_value(std::move(value)) {}

  inline const std::string& HttpHeader::get_name() const {
    return m_name;
  }

  inline const std::string& HttpHeader::get_value() const {
    return m_value;
  }
}

template<>
class std::formatter<Beam::HttpHeader> :
    public std::formatter<std::string_view> {
  public:
    auto format(const Beam::HttpHeader& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        std::format("{}: {}", value.get_name(), value.get_value()), context);
    }
};

namespace Beam {
  inline std::ostream& operator <<(
      std::ostream& sink, const HttpHeader& header) {
    return sink << std::format("{}", header);
  }
}

#endif
