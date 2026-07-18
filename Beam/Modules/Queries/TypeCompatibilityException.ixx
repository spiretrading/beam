module;
#include "Prelude.hpp"

export module Beam:TypeCompatibilityException;

export namespace Beam {

  /**
   * Indicates that an Expression was constructed with incompatible DataTypes.
   */
  class TypeCompatibilityException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;

      /** Constructs a TypeCompatibilityException. */
      TypeCompatibilityException();
  };

  inline TypeCompatibilityException::TypeCompatibilityException()
    : TypeCompatibilityException("Incompatible types.") {}
}

