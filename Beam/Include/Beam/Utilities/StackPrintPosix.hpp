#ifndef BEAM_STACKPRINTPOSIX_HPP
#define BEAM_STACKPRINTPOSIX_HPP
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cxxabi.h>
#include <execinfo.h>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
  inline std::string CaptureStackPrint() {
    std::string result;
#ifdef BEAM_ENABLE_STACK_PRINT
#ifdef NDEBUG
#endif
#else
    return result;
#endif
  }
}

#endif
