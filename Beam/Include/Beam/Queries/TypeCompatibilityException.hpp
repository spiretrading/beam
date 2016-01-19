#ifndef BEAM_TYPECOMPATIBILITYEXCEPTION_HPP
#define BEAM_TYPECOMPATIBILITYEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class TypeCompatibilityException
      \brief Indicates that an Expression was constructed with incompatible
             DataTypes.
   */
  class TypeCompatibilityException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a TypeCompatibilityException.
      TypeCompatibilityException();

      //! Constructs a TypeCompatibilityException.
      /*!
        \param message A message describing the error.
      */
      TypeCompatibilityException(const std::string& message);
  };

  inline TypeCompatibilityException::TypeCompatibilityException()
      : std::runtime_error("Incompatible types.") {}

  inline TypeCompatibilityException::TypeCompatibilityException(
      const std::string& message)
      : std::runtime_error(message) {}
}
}

#endif
