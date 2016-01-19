#ifndef BEAM_APPLICATIONINTERRUPT_HPP
#define BEAM_APPLICATIONINTERRUPT_HPP
#ifdef __GNUC__
  #include "Beam/Utilities/ApplicationInterruptPosix.hpp"
#elif defined WIN32
  #include "Beam/Utilities/ApplicationInterruptWin32.hpp"
#endif
#endif
