#ifndef BEAM_AUTHENTICATEDSESSION_HPP
#define BEAM_AUTHENTICATEDSESSION_HPP
#include <cassert>
#include "Beam/ServiceLocator/DirectoryEntry.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class AuthenticatedSession
      \brief Stores authentication info for a ServiceProtocolServlet Session.
   */
  class AuthenticatedSession {
    public:

      //! Constructs an AuthenticatedSession.
      AuthenticatedSession();

      //! Returns <code>true</code> iff the Channel is logged in.
      bool IsLoggedIn() const;

      //! Returns the account.
      const DirectoryEntry& GetAccount() const;

      //! Sets the account, establishing it as having logged in.
      /*!
        \param account The account to associate with this Channel.
      */
      void SetAccount(const DirectoryEntry& account);

      //! Resets the Account, logging it out.
      void ResetAccount();

    private:
      DirectoryEntry m_account;
  };

  inline AuthenticatedSession::AuthenticatedSession() {}

  inline bool AuthenticatedSession::IsLoggedIn() const {
    return m_account.m_id != -1;
  }

  inline const DirectoryEntry& AuthenticatedSession::GetAccount() const {
    return m_account;
  }

  inline void AuthenticatedSession::SetAccount(const DirectoryEntry& account) {
    assert(m_account.m_id == -1);
    m_account = account;
  }

  inline void AuthenticatedSession::ResetAccount() {
    m_account = DirectoryEntry();
  }
}
}

#endif
