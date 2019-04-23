#ifndef BEAM_PYTHON_HPP
#define BEAM_PYTHON_HPP
#include <utility>
#include <vector>
#include <boost/python.hpp>
#include "Beam/Routines/Scheduler.hpp"
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

namespace Beam {
namespace Python {

  //! Returns a PyObject that manages a pointer to an object.
  /*!
    \param object The object to manage.
    \return A PyObject managing the <i>object</i>.
  */
  template<class T>
  auto MakeManagedPointer(T* object) {
    return typename boost::python::manage_new_object::apply<T*>::type{}(object);
  }

  //! Returns a Python object that manages a pointer to an object.
  /*!
    \param object The object to manage.
    \return A Python object managing the <i>object</i>.
  */
  template<typename T>
  auto MakeManagedObject(T* object) {
    return boost::python::object{
      boost::python::detail::new_reference(MakeManagedPointer(object))};
  }

  //! Converts a Python list to an std::vector<T>.
  /*!
    \param list The list to convert.
    \return The <i>list</i> converted into a vector.
  */
  template<typename T>
  auto ToVector(const boost::python::list& list) {
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
