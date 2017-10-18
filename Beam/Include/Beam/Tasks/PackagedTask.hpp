#ifndef BEAM_PACKAGED_TASK_HPP
#define BEAM_PACKAGED_TASK_HPP
#include <tuple>
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/Tasks.hpp"

namespace Beam {
namespace Tasks {

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
      Package m_package;
      std::tuple<ParameterTypes...> m_parameters;
  };

  /*! \class PackagedTaskFactory
      \brief Implements the TaskFactory for PackagedTasks.
      \tparam PackageType The type containing all the info needed to execute
              a Task.
   */
  template<typename PackageType>
  class PackagedTaskFactory : public BasicTaskFactory<PackagedTaskFactory> {
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

      virtual std::shared_ptr<Task> Create() override final;

      virtual void PrepareContinuation(const Task& task) override final;

    private:
      Package m_package;
      std::vector<std::string> m_parameterNames;
  };

  template<typename PackageType>
  PackagedTaskFactory<PackageType>::PackagedTaskFactory(Package package,
      std::vector<std::string> parameterNames)
      : m_package{std::move(package)},
        m_parameterNames{std::move(parameterNames)} {}

  template<typename PackageType>
  std::shared_ptr<Task> PackagedTaskFactory<PackageType>::Create() {
    return nullptr;
  }

  template<typename PackageType>
  void PackagedTaskFactory<PackageType>::PrepareContinuation(
      const Task& task) {
    auto& packagedTask = static_cast<const Pa
  }
}
}

#endif
