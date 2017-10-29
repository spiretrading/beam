#ifndef BEAM_GILRELEASE_HPP
#define BEAM_GILRELEASE_HPP
#include <boost/noncopyable.hpp>
#include <boost/python/object.hpp>
#include <ceval.h>
#include <pystate.h>
#include <pythread.h>
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  /*! \class GilRelease
      \brief Releases Python's global interpreter lock.
   */
  class GilRelease : private boost::noncopyable {
    public:

      //! Constructs a GilRelease.
      GilRelease() = default;

      //! Releases the GIL.
      void lock();

      //! Reacquires the GIL.
      void unlock();

    private:
      bool m_hasGil;
      PyThreadState* m_state;
  };

  //! Wraps a blocking function into a Python callable that releases the
  //! Python GIL.
  /*!
    \param f The function to wrap.
    \param callPolicies The function's call policies.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename R, typename... Args, typename CallPolicies>
  boost::python::object BlockingFunction(R (*f)(Args...),
      CallPolicies callPolicies) {
    std::function<R (Args...)> callable = [=] (Args... args) -> R {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock(gil);
        return (*f)(std::forward<Args>(args)...);
      };
    using signature = boost::mpl::vector<R, Args...>;
    return boost::python::make_function(callable, callPolicies, signature());
  }

  //! Wraps a blocking function into a Python callable that releases the
  //! Python GIL.
  /*!
    \param f The function to wrap.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename R, typename... Args>
  boost::python::object BlockingFunction(R (*f)(Args...)) {
    return BlockingFunction(f, boost::python::default_call_policies());
  }

  //! Wraps a blocking function into a Python callable that releases the
  //! Python GIL.
  /*!
    \param f The function to wrap.
    \param callPolicies The function's call policies.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename R, typename T, typename... Args, typename CallPolicies>
  boost::python::object BlockingFunction(R (T::* f)(Args...),
      CallPolicies callPolicies) {
    std::function<R (T*, Args...)> callable =
      [=] (T* object, Args... args) -> R {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock(gil);
        return (object->*f)(std::forward<Args>(args)...);
      };
    using signature = boost::mpl::vector<R, T*, Args...>;
    return boost::python::make_function(callable, callPolicies, signature());
  }

  //! Wraps a blocking function into a Python callable that releases the
  //! Python GIL.
  /*!
    \param f The function to wrap.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename R, typename T, typename... Args>
  boost::python::object BlockingFunction(R (T::* f)(Args...)) {
    return BlockingFunction(f, boost::python::default_call_policies());
  }

  //! Wraps a blocking function into a Python callable that releases the
  //! Python GIL.
  /*!
    \param f The function to wrap.
    \param callPolicies The function's call policies.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename Q, typename R, typename T, typename... Args,
    typename CallPolicies>
  boost::python::object BlockingFunction(R (T::* f)(Args...),
      CallPolicies callPolicies) {
    std::function<R (Q*, Args...)> callable =
      [=] (Q* object, Args... args) -> R {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock(gil);
        return (object->*f)(std::forward<Args>(args)...);
      };
    using signature = boost::mpl::vector<R, Q*, Args...>;
    return boost::python::make_function(callable, callPolicies, signature());
  }

  //! Wraps a blocking function into a Python callable that releases the
  //! Python GIL.
  /*!
    \param f The function to wrap.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename Q, typename R, typename T, typename... Args>
  boost::python::object BlockingFunction(R (T::* f)(Args...)) {
    return BlockingFunction<Q>(f, boost::python::default_call_policies());
  }

  //! Wraps a blocking function into a Python callable that releases the
  //! Python GIL.
  /*!
    \param f The function to wrap.
    \param callPolicies The function's call policies.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename R, typename T, typename... Args, typename CallPolicies>
  boost::python::object BlockingFunction(R (T::* f)(Args...) const,
      CallPolicies callPolicies) {
    std::function<R (T*, Args...)> callable =
      [=] (T* object, Args... args) -> R {
        GilRelease gil;
        boost::lock_guard<GilRelease> lock(gil);
        return (object->*f)(std::forward<Args>(args)...);
      };
    using signature = boost::mpl::vector<R, T*, Args...>;
    return boost::python::make_function(callable, callPolicies, signature());
  }

  //! Wraps a blocking function into a Python callable that releases the
  //! Python GIL.
  /*!
    \param f The function to wrap.
    \return A Python callable that invokes <i>f</i>.
  */
  template<typename R, typename T, typename... Args>
  boost::python::object BlockingFunction(R (T::* f)(Args...) const) {
    return BlockingFunction(f, boost::python::default_call_policies());
  }

  inline void GilRelease::lock() {
    m_hasGil = HasGil();
    if(m_hasGil) {
      m_state = PyEval_SaveThread();
    }
  }

  inline void GilRelease::unlock() {
    if(m_hasGil) {
      PyEval_RestoreThread(m_state);
    }
  }
}
}

#endif
