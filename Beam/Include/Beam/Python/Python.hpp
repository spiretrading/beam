#ifndef BEAM_PYTHON_HPP
#define BEAM_PYTHON_HPP
#include <utility>
#include <vector>
#include <boost/python/list.hpp>
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

  //! Converts a Python list to an std::vector<T>.
  /*!
    \param list The list to convert.
    \return The <i>list</i> converted into a vector.
  */
  template<typename T>
  std::vector<T> ToVector(const boost::python::list& list) {
    std::vector<T> result;
    for(int i = 0; i < boost::python::len(list); ++i) {
      result.push_back(boost::python::extract<T>(list[i]));
    }
    return result;
  }

  //! Prints a Python error message.
  void PrintError();
}
}

#endif
