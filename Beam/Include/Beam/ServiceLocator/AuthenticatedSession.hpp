#ifndef BEAM_AUTHENTICATED_SESSION_HPP
#define BEAM_AUTHENTICATED_SESSION_HPP
#include <cassert>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/ServiceLocator/DirectoryEntry.hpp"

namespace Beam {

  /** Stores authentication info for a ServiceProtocolServlet Session. */
  class AuthenticatedSession {
    public:

      /** Constructs an AuthenticatedSession. */
      AuthenticatedSession() = default;

      AuthenticatedSession(const AuthenticatedSession& session);
      AuthenticatedSession(AuthenticatedSession&& session) noexcept;

      /** Returns <code>true</code> iff the Channel is logged in. */
      bool is_logged_in() const;

      /** Returns the account. */
      DirectoryEntry get_account() const;

      /**
       * Sets the account, establishing it as having logged in.
       * @param account The account to associate with this Channel.
       */
      void set_account(const DirectoryEntry& account);

      /** Resets the Account, logging it out. */
      void reset_account();

      AuthenticatedSession& operator =(const AuthenticatedSession& rhs);
      AuthenticatedSession& operator =(AuthenticatedSession&& rhs) noexcept;

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
      AuthenticatedSession&& session) noexcept {
    auto lock = boost::lock_guard(session.m_mutex);
    m_account = std::move(session.m_account);
  }

  inline bool AuthenticatedSession::is_logged_in() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_account.m_id != -1;
  }

  inline DirectoryEntry AuthenticatedSession::get_account() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_account;
  }

  inline void AuthenticatedSession::set_account(const DirectoryEntry& account) {
    auto lock = boost::lock_guard(m_mutex);
    assert(m_account.m_id == -1);
    m_account = account;
  }

  inline void AuthenticatedSession::reset_account() {
    auto lock = boost::lock_guard(m_mutex);
    m_account = {};
  }

  inline AuthenticatedSession& AuthenticatedSession::operator =(
      const AuthenticatedSession& rhs) {
    auto lock = boost::lock_guard(m_mutex);
    auto rhs_lock = boost::lock_guard(rhs.m_mutex);
    m_account = rhs.m_account;
    return *this;
  }

  inline AuthenticatedSession& AuthenticatedSession::operator =(
      AuthenticatedSession&& rhs) noexcept {
    auto lock = boost::lock_guard(m_mutex);
    auto rhs_lock = boost::lock_guard(rhs.m_mutex);
    m_account = std::move(rhs.m_account);
    return *this;
  }
}

#endif
