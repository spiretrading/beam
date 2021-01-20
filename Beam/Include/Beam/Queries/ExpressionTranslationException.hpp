#ifndef BEAM_EXPRESSION_TRANSLATION_EXCEPTION_HPP
#define BEAM_EXPRESSION_TRANSLATION_EXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {

  /** Signals that an error occurred while translating an Expression. */
  class ExpressionTranslationException : public std::runtime_error,
      public boost::exception {
    public:
      using std::runtime_error::runtime_error;
  };
}

#endif
