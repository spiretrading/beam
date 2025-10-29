#ifndef BEAM_PARSER_EXCEPTION_HPP
#define BEAM_PARSER_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

  /** Signals an error parsing a value. */
  class ParserException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a ParserException. */
      ParserException();
  };

  inline ParserException::ParserException()
    : ParserException("Parser error.") {}
}

#endif
