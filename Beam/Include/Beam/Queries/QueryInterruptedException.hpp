#ifndef BEAM_QUERY_INTERRUPTED_EXCEPTION_HPP
#define BEAM_QUERY_INTERRUPTED_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

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

#endif
