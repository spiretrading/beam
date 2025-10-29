#ifndef BEAM_ASSERTION_EXCEPTION_HPP
#define BEAM_ASSERTION_EXCEPTION_HPP
#include <sstream>
#include <stdexcept>
#include <boost/throw_exception.hpp>

#define BEAM_ASSERT(condition)                                                 \
  if(!(condition)) {                                                           \
    boost::throw_with_location(::Beam::AssertionException(#condition));        \
  }

#define BEAM_ASSERT_MESSAGE(condition, message)                                \
  if(!(condition)) {                                                           \
    auto ss = ::std::stringstream();                                           \
    ss << message;                                                             \
    boost::throw_with_location(::Beam::AssertionException(ss.str()));          \
  }

namespace Beam {

  /** Indicates a required assertion failed. */
  class AssertionException : public std::runtime_error {
    public:
      using std::runtime_error::runtime_error;
  };
}

#endif
