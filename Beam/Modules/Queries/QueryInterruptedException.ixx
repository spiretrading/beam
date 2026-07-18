module;
#include "Prelude.hpp"

export module Beam:QueryInterruptedException;

export namespace Beam {

  /** Signals that a query was interrupted. */
  class QueryInterruptedException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a QueryInterruptedException. */
      QueryInterruptedException();
  };

  inline QueryInterruptedException::QueryInterruptedException()
    : QueryInterruptedException("Query interrupted.") {}
}

