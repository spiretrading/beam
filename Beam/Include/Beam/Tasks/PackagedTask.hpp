#ifndef BEAM_PACKAGED_TASK_HPP
#define BEAM_PACKAGED_TASK_HPP
#include <tuple>
#include <boost/function_types/parameter_types.hpp>
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/Tasks.hpp"
#include "Beam/Utilities/ApplyTuple.hpp"

namespace Beam {
namespace Tasks {
namespace Details {
  template<typename T>
  struct ToTuple {};

  template<template<typename... T> class Vector, typename U, typename... V>
  struct ToTuple<Vector<U, V...>> {
    using type = std::tuple<V...>;
  };

  template<typename Package>
  struct GetParameters {
    using type = typename ToTuple<
      typename boost::function_types::parameter_types<
      decltype(&Package::Execute)>::type>::type;
  };

  template<typename ParameterTuple, typename Factory, std::size_t... I>
  decltype(auto) ParametersToTupleHelper(const Factory& f,
      std::index_sequence<I...>) {
    return std::make_tuple(
      f.Get<typename std::tuple_element<I, ParameterTuple>::type>(
      f.GetParameterName(I))...);
  }

  template<typename ParameterTuple, typename Factory, typename Indices =
    std::make_index_sequence<std::tuple_size<ParameterTuple>::value>>
  decltype(auto) ParametersToTuple(const Factory& f) {
    return ParametersToTupleHelper<ParameterTuple>(f, Indices{});
  }

  template<typename ParameterTuple, std::size_t I>
  struct DefineParameter {
    template<typename Factory>
    void operator ()(Factory& f) {
      f.DefineProperty<
        typename std::tuple_element<I - 1, ParameterTuple>::type>(
        f.GetParameterName(I - 1));
      DefineParameter<ParameterTuple, I - 1>{}(f);
    }
  };

  template<typename ParameterTuple>
  struct DefineParameter<ParameterTuple, 0> {
    template<typename Factory>
    void operator ()(Factory& f) {}
  };
}

  /*! \class PackagedTask
      \brief A Task that takes in a packaged structure containing all the
             state and functions needed to execute a Task.
      \tparam PackageType The type containing all the info needed to execute
              a Task.
   */
  template<typename PackageType>
  class PackagedTask : public BasicTask {
    public:

      //! The type containing all the info needed to execute a Task.
      using Package = PackageType;

      //! A tuple representing the parameters to pass to the Task.
      using Parameters = typename Details::GetParameters<PackageType>::type;

      //! Constructs a PackagedTask.
      /*!
        \param package The package used to execute the Task.
        \param parameters The parameters to pass to the package.
      */
      template<typename... ParameterForwards>
      PackagedTask(Package package, ParameterForwards&&... parameters);

    protected:
      virtual void OnExecute() override final;

      virtual void OnCancel() override final;

    private:
      friend class PackagedTaskFactory<Package>;
      Package m_package;
      Parameters m_parameters;
      Routines::RoutineHandler m_executeRoutine;
      Routines::RoutineHandler m_cancelRoutine;
  };

  /*! \class PackagedTaskFactory
      \brief Implements the TaskFactory for PackagedTasks.
      \tparam PackageType The type containing all the info needed to execute
              a Task.
   */
  template<typename PackageType>
  class PackagedTaskFactory :
      public BasicTaskFactory<PackagedTaskFactory<PackageType>> {
    public:

      //! The type containing all the info needed to execute a Task.
      using Package = PackageType;

      //! Constructs a PackagedTaskFactory.
      /*!
        \param package The package used to execute the Task.
        \param parameterNames The names of each parameter to the <i>package</i>.
      */
      PackagedTaskFactory(Package package,
        std::vector<std::string> parameterNames);

      //! Returns the name of a parameter.
      /*!
        \param i The index of the parameter.
        \return The name of the parameter at index <i>i</i>.
      */
      const std::string& GetParameterName(int i) const;

      virtual std::shared_ptr<Task> Create() override final;

      virtual void PrepareContinuation(const Task& task) override final;

    private:
      template<typename, std::size_t> friend struct Details::DefineParameter;
      Package m_package;
      std::vector<std::string> m_parameterNames;
  };

  template<typename PackageType>
  template<typename... ParameterForwards>
  PackagedTask<PackageType>::PackagedTask(Package package,
      ParameterForwards&&... parameters)
      : m_package{std::move(package)},
        m_parameters(std::forward<ParameterForwards>(parameters)...) {}

  template<typename PackageType>
  void PackagedTask<PackageType>::OnExecute() {
    m_executeRoutine =
      Routines::Spawn([=] {
        try {
          Apply(m_parameters,
            [&] (auto&&... parameters) {
              m_package.Execute(
                std::forward<decltype(parameters)>(parameters)...);
            });
        } catch(const std::exception& e) {
          SetTerminal(Task::State::FAILED, e.what());
          return;
        }
        SetTerminal();
      });
  }

  template<typename PackageType>
  void PackagedTask<PackageType>::OnCancel() {
    m_cancelRoutine =
      Routines::Spawn([=] {
        try {
          m_package.Cancel();
        } catch(const std::exception& e) {
          m_executeRoutine.Wait();
          SetTerminal(Task::State::FAILED, e.what());
          return;
        }
        m_executeRoutine.Wait();
        SetTerminal();
      });
  }

  template<typename PackageType>
  PackagedTaskFactory<PackageType>::PackagedTaskFactory(Package package,
      std::vector<std::string> parameterNames)
      : m_package{std::move(package)},
        m_parameterNames{std::move(parameterNames)} {
    using Parameters = typename PackagedTask<Package>::Parameters;
    Details::DefineParameter<Parameters, std::tuple_size<Parameters>::value>{}(
      *this);
  }

  template<typename PackageType>
  const std::string& PackagedTaskFactory<PackageType>::
      GetParameterName(int i) const {
    return m_parameterNames[i];
  }

  template<typename PackageType>
  std::shared_ptr<Task> PackagedTaskFactory<PackageType>::Create() {
    using Parameters = typename PackagedTask<Package>::Parameters;
    return Apply(Details::ParametersToTuple<Parameters>(*this),
      [&] (auto&&... parameters) {
        return std::make_shared<PackagedTask<Package>>(m_package,
          std::forward<decltype(parameters)>(parameters)...);
      });
  }

  template<typename PackageType>
  void PackagedTaskFactory<PackageType>::PrepareContinuation(
      const Task& task) {
    auto& packagedTask = static_cast<const PackagedTask<Package>&>(task);
    m_package = packagedTask.m_package;
  }
}
}

#endif
