#ifndef BEAM_ASSERTIONEXCEPTION_HPP
#define BEAM_ASSERTIONEXCEPTION_HPP
#include <sstream>
#include <stdexcept>
#include <boost/exception/exception.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Utilities/Utilities.hpp"

#define BEAM_ASSERT(condition)                                                 \
  if(!(condition)) {                                                           \
    BOOST_THROW_EXCEPTION(::Beam::AssertionException(#condition));             \
  }

#define BEAM_ASSERT_MESSAGE(condition, message)                                \
  if(!(condition)) {                                                           \
    ::std::stringstream m;                                                     \
    m << message;                                                              \
    BOOST_THROW_EXCEPTION(::Beam::AssertionException(m.str()));                \
  }

namespace Beam {

  /*! \class AssertionException
      \brief Indicates a required assertion failed.
   */
  class AssertionException : public std::runtime_error,
      public boost::exception {
    public:

      //! Constructs an AssertionException.
      AssertionException(const std::string& assertion);
  };

  inline AssertionException::AssertionException(const std::string& assertion)
    : std::runtime_error(assertion) {}
}

#endif
