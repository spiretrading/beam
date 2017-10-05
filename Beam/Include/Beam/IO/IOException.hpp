#ifndef BEAM_IOEXCEPTION_HPP
#define BEAM_IOEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/IO/IO.hpp"

namespace Beam {
namespace IO {

  /*! \class IOException
      \brief Signals that an error occurred while performing an IO operation.
   */
  class IOException : public std::runtime_error, public boost::exception {
    public:

      //! Constructs an IOException.
      IOException();

      //! Constructs an IOException.
      /*!
        \param message A message describing the error.
      */
      IOException(const std::string& message);
  };

  inline IOException::IOException()
      : std::runtime_error{"IO operation failed."} {}

  inline IOException::IOException(const std::string& message)
      : std::runtime_error{message} {}
}
}

#endif
