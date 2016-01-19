#ifndef BEAM_PYTHON_HPP
#define BEAM_PYTHON_HPP
#include <functional>
#include <utility>
#include <boost/mpl/vector.hpp>
#include <boost/python/object.hpp>
#include <boost/python/make_function.hpp>
#include "Beam/Routines/Scheduler.hpp"
#include "Beam/Utilities/DllExport.hpp"

BEAM_EXTERN template class BEAM_EXPORT_DLL
  Beam::Singleton<Beam::Routines::Details::Scheduler>;

BEAM_EXTERN template struct BEAM_EXPORT_DLL
  Beam::Routines::Details::CurrentRoutineGlobal<void>;

BEAM_EXTERN template struct BEAM_EXPORT_DLL
  Beam::Routines::Details::NextId<void>;

namespace Beam {
namespace Python {
}
}

#endif
