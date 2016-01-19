#ifndef BEAM_FUNCTIONOBJECT_HPP
#define BEAM_FUNCTIONOBJECT_HPP
#include <utility>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class FunctionObject
      \brief Wraps a function into a class.
      \tparam F The type of function to wrap.
  */
  template<typename F>
  class FunctionObject {
    public:

      //! The type of function to wrap.
      using Function = GetTryDereferenceType<F>;

      //! Constructs a FunctionObject.
      /*!
        \param function The function to wrap.
      */
      template<typename FunctionForward>
      FunctionObject(FunctionForward&& function);

      //! Returns the wrapped function.
      Function& GetFunction();

      //! Returns the wrapped function.
      const Function& GetFunction() const;

      //! Invokes the wrapped function.
      template<typename... Args>
      decltype(auto) operator()(Args&&... args) const {
        return (*m_function)(std::forward<Args>(args)...);
      }

      //! Invokes the wrapped function.
      template<typename... Args>
      decltype(auto) operator()(Args&&... args) {
        return (*m_function)(std::forward<Args>(args)...);
      }

    private:
      GetOptionalLocalPtr<F> m_function;
  };

  //! Makes a FunctionObject.
  /*!
    \param f The function to wrap.
  */
  template<typename F>
  auto MakeFunctionObject(F&& f) {
    return FunctionObject<typename std::decay<F>::type>{std::forward<F>(f)};
  }

  template<typename F>
  template<typename FunctionForward>
  FunctionObject<F>::FunctionObject(FunctionForward&& function)
      : m_function{std::forward<FunctionForward>(function)} {}

  template<typename F>
  typename FunctionObject<F>::Function& FunctionObject<F>::GetFunction() {
    return *m_function;
  }

  template<typename F>
  const typename FunctionObject<F>::Function& FunctionObject<F>::
      GetFunction() const {
    return *m_function;
  }
}

#endif
