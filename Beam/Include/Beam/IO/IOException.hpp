#ifndef BEAM_IO_EXCEPTION_HPP
#define BEAM_IO_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/IO/IO.hpp"

namespace Beam::IO {

  /** Signals that an error occurred while performing an IO operation. */
  class IOException : public std::runtime_error, public boost::exception {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs an IOException. */
      IOException();
  };

  inline IOException::IOException()
    : std::runtime_error("IO operation failed.") {}
}

#endif
