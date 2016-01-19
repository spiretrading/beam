#ifndef BEAM_MULTIREACTOR_HPP
#define BEAM_MULTIREACTOR_HPP
#include <vector>
#include "Beam/Pointers/UniquePtr.hpp"
#include "Beam/Reactors/ReactorContainer.hpp"
#include "Beam/Reactors/Reactor.hpp"
#include "Beam/Reactors/Reactors.hpp"
#include "Beam/Utilities/Expect.hpp"
#include "Beam/Utilities/Functional.hpp"

namespace Beam {
namespace Reactors {
namespace Details {
  template<typename T, typename U>
  bool IsMultiAssigned(Expect<T>& left, boost::optional<U>&& right) {
    if(right.is_initialized()) {
      left = std::move(*right);
      return true;
    }
    return false;
  }

  template<typename T, typename U>
  bool IsMultiAssigned(Expect<T>& left, U&& right) {
    left = std::move(right);
    return true;
  }

  template<typename T>
  struct MultiReactorType {
    typedef T type;
  };

  template<typename T>
  struct MultiReactorType<boost::optional<T>> {
    typedef T type;
  };
}

  /*! \class MultiReactor
      \brief Aggregates multiple generic Reactors together and applies a
             function when any of them updates.
      \tparam FunctionType The type of function to apply.
      \tparam ParameterType The type of Reactor used as a parameter.
   */
  template<typename FunctionType, typename ParameterType>
  class MultiReactor : public Reactor<typename Details::MultiReactorType<
      typename std::decay<GetResultOf<FunctionType,
      const std::vector<const BaseReactor*>&>>::type>::type> {
    public:
      typedef GetReactorType<Reactor<typename Details::MultiReactorType<
        typename std::decay<GetResultOf<FunctionType,
        const std::vector<const BaseReactor*>&>>::type>::type>> Type;

      //! The type of function to apply.
      typedef FunctionType Function;

      //! Constructs a MultiReactor.
      /*!
        \param function The function to apply.
        \param parameters The Reactor's parameters.
      */
      template<typename FunctionForward>
      MultiReactor(FunctionForward&& function,
        std::vector<ParameterType> parameters);

      virtual void Commit();

      virtual Type Eval() const;

    private:
      Function m_function;
      std::vector<std::unique_ptr<ReactorContainer<ParameterType>>>
        m_parameters;
      Expect<Type> m_value;
      std::vector<const BaseReactor*> m_updates;

      void UpdateValue(const std::vector<const BaseReactor*>& reactors);
  };

  //! Makes a MultiReactor.
  /*!
    \param function The function to apply.
    \param parameters The parameters to apply the <i>function</i> to.
  */
  template<typename Function, typename Parameter>
  std::shared_ptr<MultiReactor<typename std::decay<Function>::type, Parameter>>
      MakeMultiReactor(Function&& f, std::vector<Parameter> parameters) {
    return std::make_shared<MultiReactor<typename std::decay<Function>::type,
      Parameter>>(std::forward<Function>(f), std::move(parameters));
  }

  template<typename FunctionType, typename ParameterType>
  template<typename FunctionForward>
  MultiReactor<FunctionType, ParameterType>::MultiReactor(
      FunctionForward&& function, std::vector<ParameterType> parameters)
      : m_function(std::forward<FunctionForward>(function)) {
    if(parameters.empty()) {
      UpdateValue(std::vector<const BaseReactor*>());
      this->SetComplete();
      return;
    }
    for(auto& parameter : parameters) {
      auto container = std::make_unique<ReactorContainer<ParameterType>>(
        std::move(parameter), *this);
      auto existingReactor = std::find_if(m_parameters.begin(),
        m_parameters.end(),
        [&] (const std::unique_ptr<ReactorContainer<ParameterType>>& reactor) {
          return &reactor->GetReactor() == &container->GetReactor();
        });
      if(existingReactor == m_parameters.end()) {
        m_parameters.push_back(std::move(container));
      }
    }
  }

  template<typename FunctionType, typename ParameterType>
  void MultiReactor<FunctionType, ParameterType>::Commit() {
    if(this->IsComplete()) {
      for(const auto& parameter : m_parameters) {
        parameter->Commit();
      }
      return;
    }
    m_updates.clear();
    for(const auto& parameter : m_parameters) {
      if(parameter->Commit()) {
        m_updates.push_back(&parameter->GetReactor());
      }
    }
    if(!m_updates.empty()) {
      auto isInitialized = this->IsInitialized();
      if(!isInitialized) {
        isInitialized = true;
        for(const auto& parameter : m_parameters) {
          isInitialized = isInitialized && parameter->IsInitialized();
        }
      }
      if(isInitialized) {
        if(this->IsInitializing()) {
          std::vector<const BaseReactor*> parameters;
          std::transform(m_parameters.begin(), m_parameters.end(),
            std::back_inserter(parameters),
            [] (const std::unique_ptr<ReactorContainer<ParameterType>>&
                reactor) {
              return &reactor->GetReactor();
            });
          UpdateValue(parameters);
        } else {
          UpdateValue(m_updates);
        }
      }
    }
    auto isComplete = true;
    for(const auto& parameter : m_parameters) {
      isComplete = isComplete && parameter->IsComplete();
    }
    if(isComplete) {
      this->SetComplete();
    }
  }

  template<typename FunctionType, typename ParameterType>
  typename MultiReactor<FunctionType, ParameterType>::Type
      MultiReactor<FunctionType, ParameterType>::Eval() const {
    return m_value.Get();
  }

  template<typename FunctionType, typename ParameterType>
  void MultiReactor<FunctionType, ParameterType>::UpdateValue(
      const std::vector<const BaseReactor*>& reactors) {
    try {
      if(Details::IsMultiAssigned(m_value, m_function(reactors))) {
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
