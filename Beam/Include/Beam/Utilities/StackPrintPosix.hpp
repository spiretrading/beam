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
    const auto MAX_DEPTH = 100;
    void* stackAddresses[MAX_DEPTH];
    auto stackDepth = backtrace(stackAddresses, MAX_DEPTH);
    auto stackStrings = backtrace_symbols(stackAddresses, stackDepth);
    char buffer[1024];
    for(auto i = 1; i < stackDepth; ++i) {
      std::snprintf(buffer, sizeof(buffer), "%s\n", stackStrings[i]);
      result += buffer;
    }
    std::free(stackStrings);
#endif
#else
    return result;
#endif
  }
}

#endif
