#ifndef BEAM_NAMEDCHANNELIDENTIFIER_HPP
#define BEAM_NAMEDCHANNELIDENTIFIER_HPP
#include <ostream>
#include <string>
#include "Beam/IO/Channel.hpp"
#include "Beam/IO/IO.hpp"

namespace Beam {
namespace IO {

  /*! \class NamedChannelIdentifier
      \brief Implements a ChannelIdentifier using just a name.
   */
  class NamedChannelIdentifier {
    public:

      //! Constructs an empty NamedChannelIdentifier.
      NamedChannelIdentifier() = default;

      //! Constructs a NamedChannelIdentifier.
      /*!
        \param name The name of this identifier.
      */
      NamedChannelIdentifier(std::string name);

      //! Returns the name of this identifier.
      const std::string& GetName() const;

    private:
      std::string m_name;
  };

  inline std::ostream& operator <<(std::ostream& out,
      const NamedChannelIdentifier& identifier) {
    return out << identifier.GetName();
  }

  inline NamedChannelIdentifier::NamedChannelIdentifier(std::string name)
    : m_name{std::move(name)} {}

  inline const std::string& NamedChannelIdentifier::GetName() const {
    return m_name;
  }
}

  template<>
  struct ImplementsConcept<IO::NamedChannelIdentifier, IO::ChannelIdentifier>
    : std::true_type {};
}

#endif
