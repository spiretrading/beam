#ifndef BEAM_PYTHON_HPP
#define BEAM_PYTHON_HPP
#include <functional>
#include <utility>
#include <boost/mpl/vector.hpp>
#include <boost/python/object.hpp>
#include <boost/python/make_function.hpp>
#include <boost/python/manage_new_object.hpp>
#include <boost/python/return_value_policy.hpp>
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
  class PythonQueueWriter;

  //! Wraps a function returning a unique_ptr into a Python callable managing
  //! that object.
  /*!
    \param f The function to wrap.
    \param callPolicies The function's call policies.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename R, typename T, typename... Args, typename CallPolicies>
  boost::python::object ReleaseUniquePtr(R (T::* f)(Args...),
      CallPolicies callPolicies) {
    std::function<typename R::pointer (T*, Args...)> callable =
      [=] (T* object, Args... args) -> typename R::pointer {
        return (object->*f)(std::forward<Args>(args)...).release();
      };
    using signature = boost::mpl::vector<typename R::pointer, T*, Args...>;
    return boost::python::make_function(callable, callPolicies, signature());
  }

  //! Wraps a function returning a unique_ptr into a Python callable managing
  //! that object.
  /*!
    \param f The function to wrap.
    \param callPolicies The function's call policies.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename Q, typename R, typename T, typename... Args,
    typename CallPolicies>
  boost::python::object ReleaseUniquePtr(R (T::* f)(Args...),
      CallPolicies callPolicies) {
    std::function<typename R::pointer (Q*, Args...)> callable =
      [=] (Q* object, Args... args) -> typename R::pointer {
        return (object->*f)(std::forward<Args>(args)...).release();
      };
    using signature = boost::mpl::vector<typename R::pointer, Q*, Args...>;
    return boost::python::make_function(callable, callPolicies, signature());
  }

  //! Wraps a function returning a unique_ptr into a Python callable managing
  //! that object.
  /*!
    \param f The function to wrap.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename R, typename T, typename... Args>
  boost::python::object ReleaseUniquePtr(R (T::* f)(Args...)) {
    return ReleaseUniquePtr(f,
      boost::python::return_value_policy<boost::python::manage_new_object>());
  }

  //! Wraps a function returning a unique_ptr into a Python callable managing
  //! that object.
  /*!
    \param f The function to wrap.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename Q, typename R, typename T, typename... Args>
  boost::python::object ReleaseUniquePtr(R (T::* f)(Args...)) {
    return ReleaseUniquePtr<Q>(f,
      boost::python::return_value_policy<boost::python::manage_new_object>());
  }

  //! Prints a Python error message.
  void PrintError();
}
}

#endif
