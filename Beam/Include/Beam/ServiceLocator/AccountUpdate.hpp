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

    /** Tests if two updates represent the same account and type. */
    bool operator ==(const AccountUpdate& update) const;

    /** Tests if two updates have different accounts or types. */
    bool operator !=(const AccountUpdate& update) const;
  };

  inline std::ostream& operator <<(std::ostream& out,
      AccountUpdate::Type type) {
    if(type == AccountUpdate::Type::ADDED) {
      return out << "ADDED";
    }
    return out << "DELETED";
  }

  inline std::ostream& operator <<(std::ostream& out,
      const AccountUpdate& update) {
    return out << '(' << update.m_account << ' ' << update.m_type << ')';
  }

  inline bool AccountUpdate::operator ==(const AccountUpdate& update) const {
    return m_account == update.m_account && m_type == update.m_type;
  }

  inline bool AccountUpdate::operator !=(const AccountUpdate& update) const {
    return !(*this == update);
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
