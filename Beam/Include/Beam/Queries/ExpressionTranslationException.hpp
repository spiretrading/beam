#ifndef BEAM_EXPRESSION_TRANSLATION_EXCEPTION_HPP
#define BEAM_EXPRESSION_TRANSLATION_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

  /** Signals that an error occurred while translating an Expression. */
  class ExpressionTranslationException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };
}

#endif
