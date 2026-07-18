module;
#include "Prelude.hpp"
#ifdef __GNUC__
  #include "Beam/Utilities/ApplicationInterruptPosix.inl"
#elif defined WIN32
  #include "Beam/Utilities/ApplicationInterruptWin32.inl"
#endif

export module Beam:ApplicationInterrupt;

export namespace Beam {
  using Beam::is_running;
  using Beam::received_kill_event;
  using Beam::wait_for_kill_event;
}
