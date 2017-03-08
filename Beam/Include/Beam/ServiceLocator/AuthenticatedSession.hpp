#ifndef BEAM_AUTHENTICATEDSESSION_HPP
#define BEAM_AUTHENTICATEDSESSION_HPP
#include <cassert>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/ServiceLocator/DirectoryEntry.hpp"

namespace Beam {
namespace ServiceLocator {

  /*! \class AuthenticatedSession
      \brief Stores authentication info for a ServiceProtocolServlet Session.
   */
  class AuthenticatedSession {
    public:

      //! Constructs an AuthenticatedSession.
      AuthenticatedSession() = default;

      //! Copies an AuthenticatedSession.
      AuthenticatedSession(const AuthenticatedSession& session);

      //! Acquires an AuthenticatedSession.
      AuthenticatedSession(AuthenticatedSession&& session);

      AuthenticatedSession& operator =(const AuthenticatedSession& rhs);

      AuthenticatedSession& operator =(AuthenticatedSession&& rhs);

      //! Returns <code>true</code> iff the Channel is logged in.
      bool IsLoggedIn() const;

      //! Returns the account.
      DirectoryEntry GetAccount() const;

      //! Sets the account, establishing it as having logged in.
      /*!
        \param account The account to associate with this Channel.
      */
      void SetAccount(const DirectoryEntry& account);

      //! Resets the Account, logging it out.
      void ResetAccount();

    private:
      mutable boost::mutex m_mutex;
      DirectoryEntry m_account;
  };

  inline AuthenticatedSession::AuthenticatedSession(
      const AuthenticatedSession& session) {
    boost::lock_guard<boost::mutex> lock{session.m_mutex};
    m_account = session.m_account;
  }

  inline AuthenticatedSession::AuthenticatedSession(
      AuthenticatedSession&& session) {
    boost::lock_guard<boost::mutex> lock{session.m_mutex};
    m_account = std::move(session.m_account);
  }

  inline AuthenticatedSession& AuthenticatedSession::operator =(
      const AuthenticatedSession& rhs) {
    boost::lock_guard<boost::mutex> lock{rhs.m_mutex};
    m_account = rhs.m_account;
    return *this;
  }

  inline AuthenticatedSession& AuthenticatedSession::operator =(
      AuthenticatedSession&& rhs) {
    boost::lock_guard<boost::mutex> lock{rhs.m_mutex};
    m_account = std::move(rhs.m_account);
    return *this;
  }

  inline bool AuthenticatedSession::IsLoggedIn() const {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    return m_account.m_id != -1;
  }

  inline DirectoryEntry AuthenticatedSession::GetAccount() const {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    return m_account;
  }

  inline void AuthenticatedSession::SetAccount(const DirectoryEntry& account) {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    assert(m_account.m_id == -1);
    m_account = account;
  }

  inline void AuthenticatedSession::ResetAccount() {
    boost::lock_guard<boost::mutex> lock{m_mutex};
    m_account = DirectoryEntry();
  }
}
}

#endif
