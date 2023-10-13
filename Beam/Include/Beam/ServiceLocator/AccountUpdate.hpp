#ifndef BEAM_SERVICE_LOCATOR_ACCOUNT_UPDATE_HPP
#define BEAM_SERVICE_LOCATOR_ACCOUNT_UPDATE_HPP
#include <ostream>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/ServiceLocator/ServiceLocator.hpp"

namespace Beam::ServiceLocator {

  /** Stores whether an account was added or deleted. */
  struct AccountUpdate {

    /** Specifies whether the account was added or deleted. */
    enum class Type {

      /** The account was added. */
      ADDED,

      /** The account was deleted. */
      DELETED
    };

    /** The updated account. */
    DirectoryEntry m_account;

    /** The type of update. */
    Type m_type;

    bool operator ==(const AccountUpdate& update) const = default;
  };

  inline std::ostream& operator <<(std::ostream& out,
      AccountUpdate::Type type) {
    if(type == AccountUpdate::Type::ADDED) {
      return out << "ADDED";
    }
    return out << "DELETED";
  }

  inline std::ostream& operator <<(
      std::ostream& out, const AccountUpdate& update) {
    return out << '(' << update.m_account << ' ' << update.m_type << ')';
  }
}

namespace Beam::Serialization {
  template<>
  struct Shuttle<ServiceLocator::AccountUpdate> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, ServiceLocator::AccountUpdate& value,
        unsigned int version) const {
      shuttle.Shuttle("account", value.m_account);
      shuttle.Shuttle("type", value.m_type);
    }
  };
}

#endif
