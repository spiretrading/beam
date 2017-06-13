#ifndef BEAM_STACKPRINTPOSIX_HPP
#define BEAM_STACKPRINTPOSIX_HPP
#include <cxxabi.h>
#include <libunwind.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
  inline std::string CaptureStackPrint() {
    unw_cursor_t cursor;
    unw_context_t context;
    unw_getcontext(&context);
    unw_init_local(&cursor, &context);
    char buffer[1024];
    std::string result;
    while(unw_step(&cursor) > 0) {
      unw_word_t offset;
      unw_word_t pc;
      unw_get_reg(&cursor, UNW_REG_IP, &pc);
      if(pc == 0) {
        break;
      }
      std::snprintf(buffer, sizeof(buffer), "0x%lx:", pc);
      result += buffer;
      char sym[256];
      if(unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
        char* nameptr = sym;
        int status;
        char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
        if(status == 0) {
          nameptr = demangled;
        }
        std::snprintf(buffer, sizeof(buffer), " (%s+0x%lx)\n", nameptr, offset);
        result += buffer;
        std::free(demangled);
      } else {
        std::snprintf(buffer, sizeof(buffer),
          " -- error: unable to obtain symbol name for this frame\n");
        result += buffer;
      }
    }
    return result;
  }
}

#endif
