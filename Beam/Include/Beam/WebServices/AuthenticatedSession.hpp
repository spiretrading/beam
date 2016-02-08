#ifndef BEAM_WEBSERVICES_AUTHENTICATEDSESSION_HPP
#define BEAM_WEBSERVICES_AUTHENTICATEDSESSION_HPP
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/WebServices/Session.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class AuthenticatedSession
      \brief Represents a Session associated with an account.
   */
  class AuthenticatedSession : public Session {
    public:

      //! Constructs an AuthenticatedSession.
      /*!
        \param id The session's id.
      */
      AuthenticatedSession(std::string id);

      //! Returns <code>true</code> iff the Channel is logged in.
      bool IsLoggedIn() const;

      //! Returns the account.
      const ServiceLocator::DirectoryEntry& GetAccount() const;

      //! Sets the account, establishing it as having logged in.
      /*!
        \param account The account to associate with this Channel.
      */
      void SetAccount(ServiceLocator::DirectoryEntry account);

      //! Resets the Account, logging it out.
      void ResetAccount();

    private:
      ServiceLocator::DirectoryEntry m_account;
  };

  inline AuthenticatedSession::AuthenticatedSession(std::string id)
      : Session{std::move(id)} {}

  inline bool AuthenticatedSession::IsLoggedIn() const {
    return m_account.m_id != -1;
  }

  inline const ServiceLocator::DirectoryEntry&
      AuthenticatedSession::GetAccount() const {
    return m_account;
  }

  inline void AuthenticatedSession::SetAccount(
      ServiceLocator::DirectoryEntry account) {
    assert(m_account.m_id == -1);
    m_account = std::move(account);
  }

  inline void AuthenticatedSession::ResetAccount() {
    m_account = ServiceLocator::DirectoryEntry();
  }
}
}

#endif
