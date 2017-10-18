#ifndef BEAM_FUNCTION_REACTOR_HPP
#define BEAM_FUNCTION_REACTOR_HPP
#include <tuple>
#include <type_traits>
#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/algorithm/iteration/accumulate.hpp>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/LocalPtr.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/ReactorUnavailableException.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/Expect.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  template<typename T>
  struct FunctionReactorType {
    using type = T;
  };

  template<typename T>
  struct FunctionReactorType<boost::optional<T>> {
    using type = T;
  };

  struct IsComplete {
    using result_type = bool;

    template<typename T>
    bool operator ()(bool state, const T& reactor) const {
      return state && reactor->IsComplete();
    }
  };

  struct Commit {
    using result_type = BaseReactor::Update;
    int m_sequenceNumber;

    template<typename T>
    BaseReactor::Update operator ()(BaseReactor::Update update,
        T& reactor) const {
      auto reactorUpdate = reactor->Commit(m_sequenceNumber);
      if(reactorUpdate == BaseReactor::Update::NONE) {
        return update;
      } else if(reactorUpdate == BaseReactor::Update::COMPLETE) {
        if(update == BaseReactor::Update::NONE ||
            update == BaseReactor::Update::COMPLETE) {
          return reactorUpdate;
        } else {
          return update;
        }
      }
      return reactorUpdate;
    }
  };

  struct FunctionEval {
    template<typename T>
    auto operator ()(const Reactor<T>* reactor) const {
      return reactor->Eval();
    }

    Expect<void> operator ()(const BaseReactor* reactor) const {

      // TODO: Find a way to evaluate this.
      return {};
    }
  };

  template<typename T>
  struct FunctionUpdateEval {
    template<typename V, typename F, typename P>
    bool operator ()(V& value, F& function, const P& p) const {
      auto update = Apply(p,
        [&] (const auto&... parameters) {
          return boost::optional<T>{function(
            Try(std::bind(FunctionEval{}, &*parameters))...)};
        });
      if(update.is_initialized()) {
        value = std::move(*update);
        return true;
      }
      return false;
    }
  };

  template<>
  struct FunctionUpdateEval<void> {
    template<typename V, typename F, typename P>
    bool operator ()(V& value, F& function, const P& p) const {
      Apply(p,
        [&] (const auto&... parameters) {
          function(Try(std::bind(FunctionEval{}, &*parameters))...);
        });
      return true;
    }
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
      using Type = typename Reactor<typename Details::FunctionReactorType<
        typename std::decay<GetResultOf<FunctionType,
        const Expect<GetReactorType<ParameterTypes>>&...>>::type>::type>::Type;

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

      virtual bool IsComplete() const override;

      virtual BaseReactor::Update Commit(int sequenceNumber) override;

      virtual Type Eval() const override;

    private:
      Function m_function;
      std::tuple<GetOptionalLocalPtr<ParameterTypes>...> m_parameters;
      boost::optional<Expect<Type>> m_value;
      BaseReactor::Update m_state;
      BaseReactor::Update m_update;
      int m_currentSequenceNumber;

      bool AreParametersComplete() const;
      bool UpdateEval();
  };

  //! Makes a FunctionReactor.
  /*!
    \param function The function to apply.
    \param parameters The parameters to apply the <i>function</i> to.
  */
  template<typename Function, typename... Parameters>
  auto MakeFunctionReactor(Function&& f, Parameters&&... parameters) {
    return std::make_shared<FunctionReactor<typename std::decay<Function>::type,
      typename std::decay<Parameters>::type...>>(std::forward<Function>(f),
      std::forward<Parameters>(parameters)...);
  }

  template<typename FunctionType, typename... ParameterTypes>
  template<typename FunctionForward, typename... ParameterForwards>
  FunctionReactor<FunctionType, ParameterTypes...>::FunctionReactor(
      FunctionForward&& function, ParameterForwards&&... parameters)
      : m_function{std::forward<FunctionForward>(function)},
        m_parameters{std::forward<ParameterForwards>(parameters)...},
        m_state{BaseReactor::Update::NONE},
        m_currentSequenceNumber{-1} {}

  template<typename FunctionType, typename... ParameterTypes>
  bool FunctionReactor<FunctionType, ParameterTypes...>::IsComplete() const {
    return m_state == BaseReactor::Update::COMPLETE;
  }

  template<typename FunctionType, typename... ParameterTypes>
  BaseReactor::Update FunctionReactor<FunctionType, ParameterTypes...>::Commit(
      int sequenceNumber) {
    if(sequenceNumber == m_currentSequenceNumber) {
      return m_update;
    }
    if(sizeof...(ParameterTypes) == 0) {
      if(sequenceNumber == 0) {
        m_update = BaseReactor::Update::EVAL;
      } else {
        m_update = BaseReactor::Update::NONE;
      }
    } else {
      m_update = boost::fusion::accumulate(m_parameters,
        BaseReactor::Update::NONE, Details::Commit{sequenceNumber});
    }
    if(m_update == BaseReactor::Update::EVAL) {
      auto hasEval = UpdateEval();
      if(AreParametersComplete()) {
        m_state = BaseReactor::Update::COMPLETE;
      }
      if(!hasEval) {
        if(sizeof...(ParameterTypes) == 0) {
          m_update = BaseReactor::Update::COMPLETE;
        } else {
          m_update = BaseReactor::Update::NONE;
        }
      }
    } else if(m_update == BaseReactor::Update::COMPLETE) {
      if(AreParametersComplete()) {
        m_state = BaseReactor::Update::COMPLETE;
      } else {
        m_update = BaseReactor::Update::NONE;
      }
    }
    m_currentSequenceNumber = sequenceNumber;
    return m_update;
  }

  template<typename FunctionType, typename... ParameterTypes>
  typename FunctionReactor<FunctionType, ParameterTypes...>::Type
      FunctionReactor<FunctionType, ParameterTypes...>::Eval() const {
    if(m_value.is_initialized()) {
      return m_value->Get();
    }
    BOOST_THROW_EXCEPTION(ReactorUnavailableException{});
  }

  template<typename FunctionType, typename... ParameterTypes>
  bool FunctionReactor<FunctionType, ParameterTypes...>::
      AreParametersComplete() const {
    return boost::fusion::accumulate(m_parameters, true, Details::IsComplete());
  }

  template<typename FunctionType, typename... ParameterTypes>
  bool FunctionReactor<FunctionType, ParameterTypes...>::UpdateEval() {
    try {
      return Details::FunctionUpdateEval<Type>{}(m_value, m_function,
        m_parameters);
    } catch(const std::exception&) {
      m_value = std::current_exception();
      return true;
    }
  }
}
}

#endif
