#ifndef BEAM_TASKPROPERTYNOTFOUNDEXCEPTION_HPP
#define BEAM_TASKPROPERTYNOTFOUNDEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Tasks/Tasks.hpp"

namespace Beam {
namespace Tasks {

  /*! \class TaskPropertyNotFoundException
      \brief Indicates a property was not found in a TaskFactory's
             property lookup.
   */
  class TaskPropertyNotFoundException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a TaskPropertyNotFoundException.
      TaskPropertyNotFoundException();

      //! Constructs a TaskPropertyNotFoundException.
      /*!
        \param message A message describing the error.
      */
      TaskPropertyNotFoundException(const std::string& message);
  };

  inline TaskPropertyNotFoundException::TaskPropertyNotFoundException()
      : std::runtime_error{"Task property not found."} {}

  inline TaskPropertyNotFoundException::TaskPropertyNotFoundException(
      const std::string& message)
      : std::runtime_error{message} {}
}
}

#endif
