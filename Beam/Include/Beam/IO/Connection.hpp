#ifndef BEAM_CONNECTION_HPP
#define BEAM_CONNECTION_HPP
#include "Beam/IO/IO.hpp"
#include "Beam/Utilities/Concept.hpp"

namespace Beam {
namespace IO {

  /*! \struct Connection
      \brief Specifies a connection based IO resource.
   */
  struct Connection : Concept<Connection> {

    //! Closes the existing Connection.
    /*!
      \param result The result of the operation.
    */
    void Close();
  };
}
}

#endif
