#ifndef BEAM_EXPRESSIONTRANSLATIONEXCEPTION_HPP
#define BEAM_EXPRESSIONTRANSLATIONEXCEPTION_HPP
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include "Beam/Queries/Queries.hpp"

namespace Beam {
namespace Queries {

  /*! \class ExpressionTranslationException
      \brief Signals that an error occurred while translating an Expression.
   */
  class ExpressionTranslationException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs a ExpressionTranslationException.
      /*!
        \param message A message describing the error.
      */
      ExpressionTranslationException(const std::string& message);
  };

  inline ExpressionTranslationException::ExpressionTranslationException(
      const std::string& message)
      : std::runtime_error(message) {}
}
}

#endif
