#ifndef BEAM_PYTHONBINDINGS_HPP
#define BEAM_PYTHONBINDINGS_HPP
#include <memory>
#include "Beam/Network/Network.hpp"
#include "Beam/Python/Python.hpp"
#include "Beam/Threading/Threading.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam {
namespace Python {

  //! Returns the default SocketThreadPool used by Beam objects.
  BEAM_EXPORT_DLL Network::SocketThreadPool* GetSocketThreadPool();

  //! Returns the default TimerThreadPool used by Beam objects.
  BEAM_EXPORT_DLL Threading::TimerThreadPool* GetTimerThreadPool();
}
}

#endif
