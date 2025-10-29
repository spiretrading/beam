#ifndef BEAM_SERVICE_LOCATOR_ACCOUNT_UPDATE_HPP
#define BEAM_SERVICE_LOCATOR_ACCOUNT_UPDATE_HPP
#include <ostream>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"

namespace Beam {

  /** Stores whether an account was added or deleted. */
  struct AccountUpdate {

    /** Specifies whether the account was added or deleted. */
    enum class Type {

      /** The account was added. */
      ADDED,

      /** The account was deleted. */
      DELETED
    };

    /**
     * Constructs an AccountUpdate to add an account.
     * @param account The account to add.
     */
    static AccountUpdate add(DirectoryEntry account);

    /**
     * Constructs an AccountUpdate to delete an account.
     * @param account The account to delete.
     */
    static AccountUpdate remove(DirectoryEntry account);

    /** The updated account. */
    DirectoryEntry m_account;

    /** The type of update. */
    Type m_type;

    bool operator ==(const AccountUpdate&) const = default;
  };

  inline std::ostream& operator <<(
      std::ostream& out, AccountUpdate::Type type) {
    if(type == AccountUpdate::Type::ADDED) {
      return out << "ADDED";
    }
    return out << "DELETED";
  }

  inline std::ostream& operator <<(
      std::ostream& out, const AccountUpdate& update) {
    return out << '(' << update.m_account << ' ' << update.m_type << ')';
  }

  inline AccountUpdate AccountUpdate::add(DirectoryEntry account) {
    return AccountUpdate(std::move(account), Type::ADDED);
  }

  inline AccountUpdate AccountUpdate::remove(DirectoryEntry account) {
    return AccountUpdate(std::move(account), Type::DELETED);
  }

  template<>
  struct Shuttle<AccountUpdate> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, AccountUpdate& value, unsigned int version) const {
      shuttle.shuttle("account", value.m_account);
      shuttle.shuttle("type", value.m_type);
    }
  };
}

#endif
