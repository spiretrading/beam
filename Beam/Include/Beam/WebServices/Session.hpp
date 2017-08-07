#ifndef BEAM_HTTPSESSION_HPP
#define BEAM_HTTPSESSION_HPP
#include <atomic>
#include <string>
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam {
namespace WebServices {

  /*! \class Session
      \brief Base class for an HTTP session.
   */
  class Session {
    public:

      //! Constructs a Session.
      /*!
        \param id The session's id.
      */
      Session(std::string id);

      //! Returns the session id.
      const std::string& GetId() const;

      //! Returns <code>true</code> iff the session has expired.
      bool IsExpired() const;

      //! Sets the state of this session to expired.
      void SetExpired();

    protected:
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Serialization::DataShuttle;
      std::string m_id;
      std::atomic_bool m_isExpired;
  };

  inline Session::Session(std::string id)
      : m_id{std::move(id)},
        m_isExpired{false} {}

  inline const std::string& Session::GetId() const {
    return m_id;
  }

  inline bool Session::IsExpired() const {
    return m_isExpired;
  }

  inline void Session::SetExpired() {
    m_isExpired = true;
  }

  template<typename Shuttler>
  void Session::Shuttle(Shuttler& shuttle, unsigned int version) {
    shuttle.Shuttle("id", m_id);
  }
}
}

#endif
