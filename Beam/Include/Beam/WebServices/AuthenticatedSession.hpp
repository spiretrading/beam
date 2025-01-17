#ifndef BEAM_WEB_SERVICES_AUTHENTICATED_SESSION_HPP
#define BEAM_WEB_SERVICES_AUTHENTICATED_SESSION_HPP
#include <mutex>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/ServiceLocator/DirectoryEntry.hpp"
#include "Beam/WebServices/Session.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /** Represents a Session associated with an account. */
  class AuthenticatedSession : public Session {
    public:

      /**
       * Constructs an AuthenticatedSession.
       * @param id The session's id.
       */
      explicit AuthenticatedSession(std::string id);

      /** Returns <code>true</code> iff the Channel is logged in. */
      bool IsLoggedIn() const;

      /** Returns the account. */
      ServiceLocator::DirectoryEntry GetAccount() const;

      /**
       * Sets the account, establishing it as having logged in.
       * @param account The account to associate with this Channel.
       */
      void SetAccount(ServiceLocator::DirectoryEntry account);

      /** Resets the Account, logging it out. */
      void ResetAccount();

    protected:
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Serialization::DataShuttle;
      mutable std::mutex m_mutex;
      ServiceLocator::DirectoryEntry m_account;
  };

  inline AuthenticatedSession::AuthenticatedSession(std::string id)
      : Session{std::move(id)} {}

  inline bool AuthenticatedSession::IsLoggedIn() const {
    auto lock = std::lock_guard(m_mutex);
    return m_account.m_id != -1;
  }

  inline ServiceLocator::DirectoryEntry
      AuthenticatedSession::GetAccount() const {
    auto lock = std::lock_guard(m_mutex);
    return m_account;
  }

  inline void AuthenticatedSession::SetAccount(
      ServiceLocator::DirectoryEntry account) {
    auto lock = std::lock_guard(m_mutex);
    assert(m_account.m_id == -1);
    m_account = std::move(account);
  }

  inline void AuthenticatedSession::ResetAccount() {
    auto lock = std::lock_guard(m_mutex);
    m_account = {};
  }

  template<typename Shuttler>
  void AuthenticatedSession::Shuttle(Shuttler& shuttle, unsigned int version) {
    auto lock = std::lock_guard(m_mutex);
    Session::Shuttle(shuttle, version);
    shuttle.Shuttle("account", m_account);
  }
}

#endif
