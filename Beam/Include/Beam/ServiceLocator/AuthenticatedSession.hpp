#ifndef BEAM_AUTHENTICATED_SESSION_HPP
#define BEAM_AUTHENTICATED_SESSION_HPP
#include <cassert>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/ServiceLocator/DirectoryEntry.hpp"

namespace Beam::ServiceLocator {

  /** Stores authentication info for a ServiceProtocolServlet Session. */
  class AuthenticatedSession {
    public:

      /** Constructs an AuthenticatedSession. */
      AuthenticatedSession() = default;

      /** Copies an AuthenticatedSession. */
      AuthenticatedSession(const AuthenticatedSession& session);

      /** Acquires an AuthenticatedSession. */
      AuthenticatedSession(AuthenticatedSession&& session);

      /** Returns <code>true</code> iff the Channel is logged in. */
      bool IsLoggedIn() const;

      /** Returns the account. */
      DirectoryEntry GetAccount() const;

      /**
       * Sets the account, establishing it as having logged in.
       * @param account The account to associate with this Channel.
       */
      void SetAccount(const DirectoryEntry& account);

      /** Resets the Account, logging it out. */
      void ResetAccount();

      AuthenticatedSession& operator =(const AuthenticatedSession& rhs);

      AuthenticatedSession& operator =(AuthenticatedSession&& rhs);

    private:
      mutable boost::mutex m_mutex;
      DirectoryEntry m_account;
  };

  inline AuthenticatedSession::AuthenticatedSession(
      const AuthenticatedSession& session) {
    auto lock = boost::lock_guard(session.m_mutex);
    m_account = session.m_account;
  }

  inline AuthenticatedSession::AuthenticatedSession(
      AuthenticatedSession&& session) {
    auto lock = boost::lock_guard(session.m_mutex);
    m_account = std::move(session.m_account);
  }

  inline bool AuthenticatedSession::IsLoggedIn() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_account.m_id != -1;
  }

  inline DirectoryEntry AuthenticatedSession::GetAccount() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_account;
  }

  inline void AuthenticatedSession::SetAccount(const DirectoryEntry& account) {
    auto lock = boost::lock_guard(m_mutex);
    assert(m_account.m_id == -1);
    m_account = account;
  }

  inline void AuthenticatedSession::ResetAccount() {
    auto lock = boost::lock_guard(m_mutex);
    m_account = {};
  }

  inline AuthenticatedSession& AuthenticatedSession::operator =(
      const AuthenticatedSession& rhs) {
    auto lock = boost::lock_guard(m_mutex);
    auto rhsLock = boost::lock_guard(rhs.m_mutex);
    m_account = rhs.m_account;
    return *this;
  }

  inline AuthenticatedSession& AuthenticatedSession::operator =(
      AuthenticatedSession&& rhs) {
    auto lock = boost::lock_guard(rhs.m_mutex);
    auto rhsLock = boost::lock_guard(rhs.m_mutex);
    m_account = std::move(rhs.m_account);
    return *this;
  }
}

#endif
