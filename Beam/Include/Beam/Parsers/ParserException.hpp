#ifndef BEAM_PARSEREXCEPTION_HPP
#define BEAM_PARSEREXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Parsers/Parsers.hpp"

namespace Beam::Parsers {

  /*! \class ParserException
      \brief Signals an error parsing a value.
   */
  class ParserException : public std::runtime_error, public boost::exception {
    public:

      //! Constructs a ParserException.
      ParserException();

      //! Constructs a ParserException.
      /*!
        \param message A message describing the error.
      */
      ParserException(const std::string& message);
  };

  inline ParserException::ParserException()
    : ParserException("Parser error.") {}

  inline ParserException::ParserException(const std::string& message)
    : std::runtime_error(message) {}
}

#endif
