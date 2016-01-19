#ifndef BEAM_ROUTINEEXCEPTION_HPP
#define BEAM_ROUTINEEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Routines/Routines.hpp"

namespace Beam {
namespace Routines {

  /*! \class RoutineException
      \brief Used to indicate an exception occured regarding a Routine.
   */
  class RoutineException : public std::runtime_error, public boost::exception {
    public:

      //! Constructs a RoutineException.
      /*!
        \param message A message describing the error.
      */
      RoutineException(const std::string& message);
  };


  inline RoutineException::RoutineException(const std::string& message)
    : std::runtime_error(message) {}
}
}

#endif
