#ifndef BEAM_STACKPRINT_HPP
#define BEAM_STACKPRINT_HPP
#ifdef __GNUC__
  #include "Beam/Utilities/StackPrintPosix.hpp"
#elif defined WIN32
  #include "Beam/Utilities/StackPrintWin32.hpp"
#endif
#endif
