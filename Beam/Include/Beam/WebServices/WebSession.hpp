#ifndef BEAM_WEB_SESSION_HPP
#define BEAM_WEB_SESSION_HPP
#include <atomic>
#include <string>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleAtomic.hpp"

namespace Beam {

  /** Base class for an HTTP session. */
  class WebSession {
    public:

      /**
       * Constructs a WebSession.
       * @param id The session's id.
       */
      explicit WebSession(std::string id) noexcept;

      /** Returns the session id. */
      const std::string& get_id() const;

      /** Returns <code>true</code> iff the session has expired. */
      bool is_expired() const;

      /** Sets the state of this session to expired. */
      void set_expired();

    protected:
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      friend struct DataShuttle;
      std::string m_id;
      std::atomic_bool m_is_expired;

      WebSession() = default;
  };

  inline WebSession::WebSession(std::string id) noexcept
    : m_id(std::move(id)),
      m_is_expired(false) {}

  inline const std::string& WebSession::get_id() const {
    return m_id;
  }

  inline bool WebSession::is_expired() const {
    return m_is_expired;
  }

  inline void WebSession::set_expired() {
    m_is_expired = true;
  }

  template<IsShuttle S>
  void WebSession::shuttle(S& shuttle, unsigned int version) {
    shuttle.shuttle("id", m_id);
    shuttle.shuttle("is_expired", m_is_expired);
  }
}

#endif
