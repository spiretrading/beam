#ifndef BEAM_CPPWORKAROUNDS_HPP
#define BEAM_CPPWORKAROUNDS_HPP
#include "Beam/Utilities/Utilities.hpp"

#ifdef __GNUC__
  #define BEAM_SUPPRESS_THIS_INITIALIZER()
  #define BEAM_UNSUPPRESS_THIS_INITIALIZER()
  #define BEAM_SUPPRESS_POD_INITIALIZER()
  #define BEAM_UNSUPPRESS_POD_INITIALIZER()
  #define BEAM_SUPPRESS_RECURSIVE_OVERFLOW()
  #define BEAM_UNSUPPRESS_RECURSIVE_OVERFLOW()
  #define BEAM_SUPPRESS_MULTIPLE_CONSTRUCTORS()
  #define BEAM_UNSUPPRESS_MULTIPLE_CONSTRUCTORS()
  #define BEAM_SUPPRESS_FORMAT_TRUNCATION()                                    \
    _Pragma("GCC diagnostic push")                                             \
    _Pragma("GCC diagnostic ignored \"-Wformat-truncation=\"")
  #define BEAM_UNSUPPRESS_FORMAT_TRUNCATION()                                  \
    _Pragma("GCC diagnostic pop")
#elif defined _MSC_VER
  #define BEAM_SUPPRESS_THIS_INITIALIZER() __pragma(warning(disable: 4355))
  #define BEAM_UNSUPPRESS_THIS_INITIALIZER() __pragma(warning(default: 4355))
  #define BEAM_SUPPRESS_POD_INITIALIZER() __pragma(warning(disable: 4345))
  #define BEAM_UNSUPPRESS_POD_INITIALIZER() __pragma(warning(default: 4345))
  #define BEAM_SUPPRESS_RECURSIVE_OVERFLOW() __pragma(warning(disable: 4717))
  #define BEAM_UNSUPPRESS_RECURSIVE_OVERFLOW() __pragma(warning(default: 4717))
  #define BEAM_SUPPRESS_MULTIPLE_CONSTRUCTORS() __pragma(warning(disable: 4521))
  #define BEAM_UNSUPPRESS_MULTIPLE_CONSTRUCTORS() __pragma(warning(default: 4521))
  #define BEAM_SUPPRESS_FORMAT_TRUNCATION()
  #define BEAM_UNSUPPRESS_FORMAT_TRUNCATION()
#else
  #define BEAM_SUPPRESS_THIS_INITIALIZER()
  #define BEAM_UNSUPPRESS_THIS_INITIALIZER()
  #define BEAM_SUPPRESS_POD_INITIALIZER()
  #define BEAM_UNSUPPRESS_POD_INITIALIZER()
  #define BEAM_SUPPRESS_RECURSIVE_OVERFLOW()
  #define BEAM_UNSUPPRESS_RECURSIVE_OVERFLOW()
  #define BEAM_SUPPRESS_MULTIPLE_CONSTRUCTORS()
  #define BEAM_UNSUPPRESS_MULTIPLE_CONSTRUCTORS()
  #define BEAM_SUPPRESS_FORMAT_TRUNCATION()
  #define BEAM_UNSUPPRESS_FORMAT_TRUNCATION()
#endif

#define BEAM_PREVENT_MACRO_SUBSTITUTION

#endif
