#ifndef BEAM_FUNCTIONREACTOR_HPP
#define BEAM_FUNCTIONREACTOR_HPP
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/algorithm/iteration/accumulate.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/optional/optional.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/ReactorContainer.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/ApplyTuple.hpp"
#include "Beam/Utilities/BeamWorkaround.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/Functional.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  struct Commit {
    using result_type = bool;

    template<typename T>
    bool operator ()(bool state, T& reactor) const {
      return reactor->Commit() || state;
    }
  };

  struct IsInitialized {
    using result_type = bool;

    template<typename T>
    bool operator ()(bool state, const T& reactor) const {
      return state && reactor->IsInitialized();
    }
  };

  struct IsComplete {
    using result_type = bool;

    template<typename T>
    bool operator ()(bool state, const T& reactor) const {
      return state && reactor->IsComplete();
    }
  };

  template<typename T, typename U>
  bool IsAssigned(Expect<T>& left, boost::optional<U>&& right) {
    if(right.is_initialized()) {
      left = std::move(*right);
      return true;
    }
    return false;
  }

  template<typename T, typename U>
  bool IsAssigned(Expect<T>& left, U&& right) {
    left = std::move(right);
    return true;
  }

  template<typename T>
  struct FunctionReactorType {
    using type = T;
  };

  template<typename T>
  struct FunctionReactorType<boost::optional<T>> {
    using type = T;
  };
}

  /*! \class FunctionReactor
      \brief A Reactor that applies a function to its parameters.
      \tparam FunctionType The type of function to apply.
      \tparam ParameterTypes The type of parameters to apply the function to.
   */
  template<typename FunctionType, typename... ParameterTypes>
  class FunctionReactor : public Reactor<typename Details::FunctionReactorType<
      typename std::decay<GetResultOf<FunctionType, const Expect<GetReactorType<
      ParameterTypes>>&...>>::type>::type> {
    public:
      using Type = typename Details::FunctionReactorType<typename std::decay<
        GetResultOf<FunctionType, const Expect<GetReactorType<
        ParameterTypes>>&...>>::type>::type;

      //! The type of function to apply.
      using Function = FunctionType;

      //! Constructs a FunctionReactor.
      /*!
        \param function The function to apply.
        \param parameters The parameters to apply the <i>function</i> to.
      */
      template<typename FunctionForward, typename... ParameterForwards>
      FunctionReactor(FunctionForward&& function,
        ParameterForwards&&... parameters);

      virtual void Commit();

      virtual Type Eval() const;

    private:
      Function m_function;
      std::tuple<LocalPtr<ReactorContainer<ParameterTypes>>...>
        m_parameters;
      Expect<Type> m_value;

      void UpdateValue();
  };

  //! Makes a FunctionReactor.
  /*!
    \param function The function to apply.
    \param parameters The parameters to apply the <i>function</i> to.
  */
  template<typename Function, typename... Parameters>
  std::shared_ptr<FunctionReactor<typename std::decay<Function>::type,
      typename std::decay<Parameters>::type...>> MakeFunctionReactor(
      Function&& f, Parameters&&... parameters) {
    return std::make_shared<FunctionReactor<typename std::decay<Function>::type,
      typename std::decay<Parameters>::type...>>(std::forward<Function>(f),
      std::forward<Parameters>(parameters)...);
  }

  template<typename FunctionType, typename... ParameterTypes>
  template<typename FunctionForward, typename... ParameterForwards>
  FunctionReactor<FunctionType, ParameterTypes...>::FunctionReactor(
      FunctionForward&& function, ParameterForwards&&... parameters)
BEAM_SUPPRESS_THIS_INITIALIZER()
      : m_function(std::forward<FunctionForward>(function)),
        m_parameters(Initialize(std::forward<ParameterForwards>(parameters),
          *this)...) {
BEAM_UNSUPPRESS_THIS_INITIALIZER()
    if(sizeof...(ParameterTypes) == 0) {
      UpdateValue();
      this->SetComplete();
    }
  }

  template<typename FunctionType, typename... ParameterTypes>
  void FunctionReactor<FunctionType, ParameterTypes...>::Commit() {
    auto ignoreUpdate = this->IsComplete();
    auto hasChange = boost::fusion::accumulate(m_parameters, false,
      Details::Commit());
    if(ignoreUpdate) {
      return;
    }
    if(hasChange) {
      auto isInitialized = this->IsInitialized() ||
        boost::fusion::accumulate(m_parameters, true, Details::IsInitialized());
      if(isInitialized) {
        UpdateValue();
      }
    }
    auto isComplete = boost::fusion::accumulate(m_parameters, true,
      Details::IsComplete());
    if(isComplete) {
      this->SetComplete();
    }
  }

  template<typename FunctionType, typename... ParameterTypes>
  typename FunctionReactor<FunctionType, ParameterTypes...>::Type
      FunctionReactor<FunctionType, ParameterTypes...>::Eval() const {
    return m_value.Get();
  }

  template<typename FunctionType, typename... ParameterTypes>
  void FunctionReactor<FunctionType, ParameterTypes...>::UpdateValue() {
    try {
      if(Details::IsAssigned(m_value, Apply(m_parameters,
          [&] (const LocalPtr<ReactorContainer<ParameterTypes>>&...
              parameters) {
            return m_function(parameters->GetValue()...);
          }))) {
        this->IncrementSequenceNumber();
      }
    } catch(const std::exception&) {
      m_value = std::current_exception();
      this->IncrementSequenceNumber();
    }
  }
}
}

#endif
