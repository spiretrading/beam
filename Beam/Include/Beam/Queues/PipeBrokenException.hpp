#ifndef BEAM_PIPEBROKENEXCEPTION_HPP
#define BEAM_PIPEBROKENEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Queues/Queues.hpp"

namespace Beam {

  /*! \class PipeBrokenException
      \brief Thrown when an operation is performed on a broken Queue.
   */
  class PipeBrokenException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a PipeBrokenException.
      PipeBrokenException();

      //! Constructs a PipeBrokenException.
      /*!
        \param message A message describing the error.
      */
      PipeBrokenException(const std::string& message);
  };

  inline PipeBrokenException::PipeBrokenException()
    : std::runtime_error("Pipe broken.") {}

  inline PipeBrokenException::PipeBrokenException(const std::string& message)
    : std::runtime_error(message) {}
}

#endif
