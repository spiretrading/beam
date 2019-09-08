#ifndef BEAM_PYTHON_HPP
#define BEAM_PYTHON_HPP
#include "Beam/Network/Network.hpp"
#include "Beam/Python/BasicTypeCaster.hpp"
#include "Beam/Python/Collections.hpp"
#include "Beam/Python/DateTime.hpp"
#include "Beam/Python/Decimal.hpp"
#include "Beam/Python/Enum.hpp"
#include "Beam/Python/EnumSet.hpp"
#include "Beam/Python/Expect.hpp"
#include "Beam/Python/FixedString.hpp"
#include "Beam/Python/IO.hpp"
#include "Beam/Python/Network.hpp"
#include "Beam/Python/Optional.hpp"
#include "Beam/Python/Out.hpp"
#include "Beam/Python/Queries.hpp"
#include "Beam/Python/Rational.hpp"
#include "Beam/Python/Reactors.hpp"
#include "Beam/Python/Ref.hpp"
#include "Beam/Python/Routines.hpp"
#include "Beam/Python/ServiceLocator.hpp"
#include "Beam/Python/Sql.hpp"
#include "Beam/Python/Threading.hpp"
#include "Beam/Python/TimeService.hpp"
#include "Beam/Python/ToPythonServiceLocatorClient.hpp"
#include "Beam/Python/ToPythonTimeClient.hpp"
#include "Beam/Python/ToPythonTimer.hpp"
#include "Beam/Python/ToPythonUidClient.hpp"
#include "Beam/Python/UidService.hpp"
#include "Beam/Python/WebServices.hpp"
#include "Beam/Python/Yaml.hpp"
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
