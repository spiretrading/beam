#ifndef BEAM_TYPE_COMPATIBILITY_EXCEPTION_HPP
#define BEAM_TYPE_COMPATIBILITY_EXCEPTION_HPP
#include <stdexcept>

namespace Beam {

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

#endif
