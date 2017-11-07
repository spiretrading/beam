#ifndef BEAM_CPPUNITWORKAROUNDS_HPP
#define BEAM_CPPUNITWORKAROUNDS_HPP
#include "Beam/Utilities/Utilities.hpp"

#ifdef __GNUC__
  #define BEAM_CPPUNIT_TEST_SUITE_END()                                        \
    _Pragma("GCC diagnostic push")                                             \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")            \
      CPPUNIT_TEST_SUITE_END();                                                \
    _Pragma("GCC diagnostic pop")
  #define BEAM_CPPUNIT_TEST_SUITE_END_ABSTRACT()                               \
    _Pragma("GCC diagnostic push")                                             \
    _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")            \
      CPPUNIT_TEST_SUITE_END_ABSTRACT();                                       \
    _Pragma("GCC diagnostic pop")
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
#elif defined _MSC_VER
  #include <exception>
  #define BEAM_CPPUNIT_TEST_SUITE_END CPPUNIT_TEST_SUITE_END
  #define BEAM_CPPUNIT_TEST_SUITE_END_ABSTRACT CPPUNIT_TEST_SUITE_END_ABSTRACT
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
  #define BEAM_CPPUNIT_TEST_SUITE_END CPPUNIT_TEST_SUITE_END
  #define BEAM_CPPUNIT_TEST_SUITE_END_ABSTRACT CPPUNIT_TEST_SUITE_END_ABSTRACT
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
