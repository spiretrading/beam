#ifndef BEAM_PYTHON_NO_THROW_FUNCTION_HPP
#define BEAM_PYTHON_NO_THROW_FUNCTION_HPP
#include <functional>
#include "Beam/Python/Python.hpp"
#include "Beam/Python/GilLock.hpp"
#include "Beam/Python/SharedObject.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename T>
  struct Extract {
    T operator ()(const boost::python::object& result) const {
      return boost::python::extract<T>{result}();
    }
  };

  template<typename T>
  struct Extract<std::unique_ptr<T>> {
    std::unique_ptr<T> operator ()(const boost::python::object& result) const {
      return std::unique_ptr<T>{boost::python::extract<T*>{result}()};
    }
  };
}

  /*! \class NoThrowFunction
      \brief Wraps a function to report any Python error.
      \tparam R The function's return type.
      \tparam Args The function's parameters.
   */
  template<typename R, typename... Args>
  class NoThrowFunction {
    public:

      //! Constructs a NoThrowFunction.
      /*!
        \param function The function to wrap.
      */
      NoThrowFunction(boost::python::object function);

      //! Calls the wrapped function.
      /*!
        \param p The parameters to pass to the function.
      */
      template<typename... P>
      auto operator ()(P&&... p) const {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        try {
          return Details::Extract<R>{}((*m_function)(std::forward<P>(p)...));
        } catch(const boost::python::error_already_set&) {
          PrintError();
          throw;
        }
      }

    private:
      SharedObject m_function;
  };

  template<typename... Args>
  class NoThrowFunction<void, Args...> {
    public:
      NoThrowFunction(boost::python::object function);

      template<typename... P>
      void operator ()(P&&... p) const {
        GilLock gil;
        boost::lock_guard<GilLock> lock{gil};
        try {
          (*m_function)(std::forward<P>(p)...);
        } catch(const boost::python::error_already_set&) {
          PrintError();
          throw;
        }
      }

    private:
      SharedObject m_function;
  };

  template<typename R, typename... Args>
  NoThrowFunction<R, Args...>::NoThrowFunction(boost::python::object function)
      : m_function{std::move(function)} {}

  template<typename... Args>
  NoThrowFunction<void, Args...>::NoThrowFunction(
      boost::python::object function)
      : m_function{std::move(function)} {}
}
}

#endif
