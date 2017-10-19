#ifndef BEAM_FUNCTION_TASK_HPP
#define BEAM_FUNCTION_TASK_HPP
#include "Beam/Tasks/PackagedTask.hpp"
#include "Beam/Tasks/Tasks.hpp"
#include "Beam/Utilities/Functional.hpp"

namespace Beam {
namespace Tasks {
namespace Details {
  template<typename F, typename... Parameters>
  struct FunctionPackage {
    std::function<void (Parameters...)> m_function;

    FunctionPackage(const std::function<void (Parameters...)>& f)
      : m_function{f} {}

    void Execute(const Parameters&... parameters) {
      m_function(parameters...);
    }

    void Cancel() {}
  };

  template<typename F, typename T>
  struct ToFunctionPackageHelper {};

  template<typename F, template<typename... T> class S, typename U,
    typename... V>
  struct ToFunctionPackageHelper<F, S<U, V...>> {
    using type = FunctionPackage<F, V...>;
  };

  template<typename F>
  struct ToFunctionPackage {
    using type = typename ToFunctionPackageHelper<F,
      GetFunctionParameters<F>>::type;
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
