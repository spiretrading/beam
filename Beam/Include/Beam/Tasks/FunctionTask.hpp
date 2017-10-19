#ifndef BEAM_FUNCTION_TASK_HPP
#define BEAM_FUNCTION_TASK_HPP
#include <boost/type_traits/is_function.hpp>
#include "Beam/Tasks/PackagedTask.hpp"
#include "Beam/Tasks/Tasks.hpp"

namespace Beam {
namespace Tasks {
namespace Details {
  template<typename F, typename... Parameters>
  struct FunctionPackage {
    F m_function;

    template<typename Forward>
    FunctionPackage(Forward&& f)
      : m_function{std::forward<Forward>(f)} {}

    FunctionPackage(const FunctionPackage&) = default;

    void Execute(Parameters&&... parameters) {
      m_function(std::forward<Parameters>(parameters)...);
    }

    void Cancel() {}
  };

  template<typename F, bool IsFunction = boost::is_function<F>::value>
  struct GetFunction {};

  template<typename F>
  struct GetFunction<F, true> {
    using type = F;
  };

  template<typename F>
  struct GetFunction<F, false> {
    using type = decltype(&F::operator ());
  };

  template<typename F, typename T>
  struct ToFunctionPackageHelper {};

  template<typename F, template<typename... T> class Vector, typename U,
    typename... V>
  struct ToFunctionPackageHelper<F, Vector<U, V...>> {
    using type = FunctionPackage<F, V...>;
  };

  template<typename F>
  struct ToFunctionPackage {
    using type = typename ToFunctionPackageHelper<F,
      typename boost::function_types::parameter_types<
      typename GetFunction<F>::type>::type>::type;
  };
}

  //! Makes a TaskFactory that executes a function.
  /*!
    \param f The function to call.
    \param parameterNames The names of the parameters.
  */
  template<typename F>
  TaskFactory MakeFunctionTaskFactory(F&& f,
      std::array<std::string, std::tuple_size<typename PackagedTask<
      typename Details::ToFunctionPackage<F>::type>::Parameters>::value>
      parameterNames) {
    using Package = typename Details::ToFunctionPackage<F>::type;
    return PackagedTaskFactory<Package>{Package{std::forward<F>(f)},
      std::move(parameterNames)};
  }
}
}

#endif
