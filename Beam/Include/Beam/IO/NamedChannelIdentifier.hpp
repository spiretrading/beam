#ifndef BEAM_NAMED_CHANNEL_IDENTIFIER_HPP
#define BEAM_NAMED_CHANNEL_IDENTIFIER_HPP
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

  inline std::ostream& operator <<(
      std::ostream& out, const NamedChannelIdentifier& identifier) {
    return out << identifier.get_name();
  }

  inline NamedChannelIdentifier::NamedChannelIdentifier(
    std::string name) noexcept
    : m_name(std::move(name)) {}

  inline const std::string& NamedChannelIdentifier::get_name() const {
    return m_name;
  }
}

#endif
