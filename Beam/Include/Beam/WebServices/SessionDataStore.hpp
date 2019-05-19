#ifndef BEAM_SESSION_DATA_STORE_HPP
#define BEAM_SESSION_DATA_STORE_HPP
#include <memory>
#include "Beam/Utilities/Concept.hpp"
#include "Beam/WebServices/Session.hpp"
#include "Beam/WebServices/WebServices.hpp"

namespace Beam::WebServices {

  /*! \class SessionDataStore
      \brief Used to store persistent Sessions.
   */
  struct SessionDataStore : Concept<SessionDataStore> {

    //! Loads a session with a specified id.
    /*!
      \param id The session id.
      \return The session with the specified <i>id</i> or <code>nullptr</code>
              if no such existing session exists.
     */
    template<typename Session>
    std::unique_ptr<Session> Load(const std::string& id);

    //! Stores a session.
    /*!
      \param session The session to store.
    */
    template<typename Session>
    void Store(const Session& session);

    //! Deletes a session.
    /*!
      \param session The session to delete.
    */
    template<typename Session>
    void Delete(const Session& session);

    //! Executes a transactional query.
    /*!
      \param transaction The transaction to perform.
      \return The result of calling <i>transaction()</i>.
     */
    template<typename F>
    void WithTransaction(F&& transaction);

    void Open();

    void Close();
  };
}

#endif
