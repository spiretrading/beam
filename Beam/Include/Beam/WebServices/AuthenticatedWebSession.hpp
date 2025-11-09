#ifndef BEAM_AUTHENTICATED_WEB_SESSION_HPP
#define BEAM_AUTHENTICATED_WEB_SESSION_HPP
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/WebServices/WebSession.hpp"

namespace Beam {

  /** Represents a Session associated with an account. */
  class AuthenticatedWebSession : public WebSession {
    public:

      /**
       * Constructs an AuthenticatedWebSession.
       * @param id The session's id.
       */
      explicit AuthenticatedWebSession(std::string id) noexcept;

      /** Returns <code>true</code> iff the Channel is logged in. */
      bool is_logged_in() const;

      /** Returns the account. */
      DirectoryEntry get_account() const;

      /**
       * Sets the account, establishing it as having logged in.
       * @param account The account to associate with this Channel.
       */
      void set_account(DirectoryEntry account);

      /** Resets the Account, logging it out. */
      void reset_account();

    protected:
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      friend struct DataShuttle;
      mutable boost::mutex m_mutex;
      DirectoryEntry m_account;

      AuthenticatedWebSession() noexcept;
  };

  inline AuthenticatedWebSession::AuthenticatedWebSession(
    std::string id) noexcept
    : WebSession(std::move(id)) {}

  inline bool AuthenticatedWebSession::is_logged_in() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_account.m_id != -1;
  }

  inline DirectoryEntry AuthenticatedWebSession::get_account() const {
    auto lock = boost::lock_guard(m_mutex);
    return m_account;
  }

  inline void AuthenticatedWebSession::set_account(DirectoryEntry account) {
    auto lock = boost::lock_guard(m_mutex);
    assert(m_account.m_id == -1);
    m_account = std::move(account);
  }

  inline void AuthenticatedWebSession::reset_account() {
    auto lock = boost::lock_guard(m_mutex);
    m_account = {};
  }

  template<IsShuttle S>
  void AuthenticatedWebSession::shuttle(S& shuttle, unsigned int version) {
    auto lock = boost::lock_guard(m_mutex);
    WebSession::shuttle(shuttle, version);
    shuttle.shuttle("account", m_account);
  }

  inline AuthenticatedWebSession::AuthenticatedWebSession() noexcept
    : WebSession("") {}
}

#endif
