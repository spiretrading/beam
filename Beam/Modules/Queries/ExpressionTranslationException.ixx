module;
#include "Prelude.hpp"

export module Beam:ExpressionTranslationException;

import :Expression;

export namespace Beam {

  /** Signals that an error occurred while translating an Expression. */
  class ExpressionTranslationException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };
}

