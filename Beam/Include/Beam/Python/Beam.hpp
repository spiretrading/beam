#ifndef BEAM_PYTHON_HPP
#define BEAM_PYTHON_HPP
#include "Beam/Network/Network.hpp"
#include "Beam/Python/Collections.hpp"
#include "Beam/Python/DateTime.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/EnumSet.hpp"
#include "Beam/Python/Reactors.hpp"
#include "Beam/Routines/Scheduler.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Utilities/DllExport.hpp"

#ifdef _MSC_VER
  __pragma(warning(disable: 4251))
  __pragma(warning(disable: 4275))
#endif

BEAM_EXTERN template class BEAM_EXPORT_DLL
  Beam::Singleton<Beam::Routines::Details::Scheduler>;

BEAM_EXTERN template struct BEAM_EXPORT_DLL
  Beam::Routines::Details::CurrentRoutineGlobal<void>;

BEAM_EXTERN template struct BEAM_EXPORT_DLL
  Beam::Routines::Details::NextId<void>;

namespace Beam::Python {

  /** Returns the default SocketThreadPool used by Beam objects. */
  BEAM_EXPORT_DLL Network::SocketThreadPool* GetSocketThreadPool();

  /** Returns the default TimerThreadPool used by Beam objects. */
  BEAM_EXPORT_DLL Threading::TimerThreadPool* GetTimerThreadPool();
}

#endif
