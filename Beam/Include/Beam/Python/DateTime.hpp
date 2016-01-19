#ifndef BEAM_PYTHONDATETIME_HPP
#define BEAM_PYTHONDATETIME_HPP
#include "Beam/Python/Python.hpp"
#include "Beam/Utilities/DllExport.hpp"

namespace Beam {
namespace Python {

  //! Exports the ptime class.
  BEAM_EXPORT_DLL void ExportPtime();

  //! Exports the time_duration class.
  BEAM_EXPORT_DLL void ExportTimeDuration();
}
}

#endif
