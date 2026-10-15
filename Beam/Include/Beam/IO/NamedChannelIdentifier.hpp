#ifndef BEAM_NAMED_CHANNEL_IDENTIFIER_HPP
#define BEAM_NAMED_CHANNEL_IDENTIFIER_HPP
#include <format>
#include "Beam/IO/ChannelIdentifier.hpp"

namespace Beam {

  /** Implements a ChannelIdentifier using just a name. */
  class NamedChannelIdentifier {
    public:

      /** Constructs an empty NamedChannelIdentifier. */
      NamedChannelIdentifier() = default;

      /**
       * Constructs a NamedChannelIdentifier.
       * @param name The name of this identifier.
       */
      explicit NamedChannelIdentifier(std::string name) noexcept;

      /** Returns the name of this identifier. */
      const std::string& get_name() const;

    private:
      std::string m_name;
  };

  inline NamedChannelIdentifier::NamedChannelIdentifier(
    std::string name) noexcept
    : m_name(std::move(name)) {}

  inline const std::string& NamedChannelIdentifier::get_name() const {
    return m_name;
  }
}

template<>
class std::formatter<Beam::NamedChannelIdentifier> :
    public std::formatter<std::string_view> {
  public:
    auto format(
        const Beam::NamedChannelIdentifier& value, auto& context) const {
      return std::formatter<std::string_view>::format(
        value.get_name(), context);
    }
};

namespace Beam {
  inline std::ostream& operator <<(
      std::ostream& out, const NamedChannelIdentifier& identifier) {
    return out << std::format("{}", identifier);
  }
}

#endif
