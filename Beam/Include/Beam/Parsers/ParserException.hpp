#ifndef BEAM_PARSER_EXCEPTION_HPP
#define BEAM_PARSER_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /** Signals an error parsing a value. */
  class ParserException : public std::runtime_error, public boost::exception {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a ParserException. */
      ParserException();
  };

  inline ParserException::ParserException()
    : ParserException("Parser error.") {}
}

#endif
