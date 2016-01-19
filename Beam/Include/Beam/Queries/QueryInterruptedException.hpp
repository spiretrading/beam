#ifndef BEAM_QUERYINTERRUPTEDEXCEPTION_HPP
#define BEAM_QUERYINTERRUPTEDEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class QueryInterruptedException
      \brief Signals that a query was interrupted.
   */
  class QueryInterruptedException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a QueryInterruptedException.
      QueryInterruptedException();

      //! Constructs a QueryInterruptedException.
      /*!
        \param message A message describing the error.
      */
      QueryInterruptedException(const std::string& message);
  };

  inline QueryInterruptedException::QueryInterruptedException()
      : std::runtime_error("Query interrupted.") {}

  inline QueryInterruptedException::QueryInterruptedException(
      const std::string& message)
      : std::runtime_error(message) {}
}
}

#endif
