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
    return result;
  }
}

#endif
