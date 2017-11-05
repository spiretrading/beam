#ifndef BEAM_TO_PYTHON_TASK_HPP
#define BEAM_TO_PYTHON_TASK_HPP
#include <boost/optional/optional.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Tasks/Tasks.hpp"
#include "Beam/Tasks/Task.hpp"

namespace Beam {
namespace Tasks {

  /*! \class ToPythonTask
      \brief Wraps a C++ Task for use within Python.
      \tparam T The Task to wrap.
   */
  template<typename T>
  class ToPythonTask : public Task {
    public:

      //! The Task being wrapped.
      using WrappedTask = T;

      //! Constructs a ToPythonTask.
      /*!
        \param args The arguments to pass to the wrapped Task's constructor.
      */
      template<typename... Args>
      ToPythonTask(Args&&... args);

      //! Constructs a ToPythonTask wrapping an existing Task.
      /*!
        \param task The Task to wrap.
      */
      ToPythonTask(std::shared_ptr<WrappedTask> task);

      virtual ~ToPythonTask() override final;

      //! Returns the Task being wrapped.
      const WrappedTask& GetTask() const;

      //! Returns the Task being wrapped.
      WrappedTask& GetTask();

      virtual void Execute() override final;

      virtual void Cancel() override final;

      virtual const Publisher<StateEntry>& GetPublisher() const override final;

    private:
      std::shared_ptr<WrappedTask> m_task;
  };

  /*! \class ToPythonTaskFactory
      \brief Wraps a C++ TaskFactory for use within Python.
      \tparam T The TaskFactory to wrap.
   */
  template<typename T>
  class ToPythonTaskFactory : public VirtualTaskFactory,
      public CloneableMixin<ToPythonTaskFactory<T>> {
    public:

      //! The TaskFactory being wrapped.
      using WrappedTaskFactory = T;

      //! Constructs a ToPythonTaskFactory.
      /*!
        \param args The arguments to pass to the wrapped TaskFactory's
               constructor.
      */
      template<typename... Args>
      ToPythonTaskFactory(Args&&... args);

      ToPythonTaskFactory(const ToPythonTaskFactory& factory) = default;

      virtual ~ToPythonTaskFactory() override final;

      virtual std::shared_ptr<Task> Create();

      virtual boost::any& FindProperty(const std::string& name);

      virtual void PrepareContinuation(const Task& task);

    private:
      boost::optional<WrappedTaskFactory> m_factory;
  };

  template<typename T>
  template<typename... Args>
  ToPythonTask<T>::ToPythonTask(Args&&... args)
      : m_task{std::make_shared<WrappedTask>(std::forward<Args>(args)...)} {}

  template<typename T>
  ToPythonTask<T>::ToPythonTask(std::shared_ptr<WrappedTask> task)
      : m_task{std::move(task)} {}

  template<typename T>
  ToPythonTask<T>::~ToPythonTask() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_task.reset();
  }

  template<typename T>
  const typename ToPythonTask<T>::WrappedTask&
      ToPythonTask<T>::GetTask() const {
    return *m_task;
  }

  template<typename T>
  typename ToPythonTask<T>::WrappedTask& ToPythonTask<T>::GetTask() {
    return *m_task;
  }

  template<typename T>
  void ToPythonTask<T>::Execute() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_task->Execute();
  }

  template<typename T>
  void ToPythonTask<T>::Cancel() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_task->Cancel();
  }

  template<typename T>
  const Publisher<Task::StateEntry>& ToPythonTask<T>::GetPublisher() const {
    return m_task->GetPublisher();
  }

  template<typename T>
  template<typename... Args>
  ToPythonTaskFactory<T>::ToPythonTaskFactory(Args&&... args) {
    m_factory.emplace(std::forward<Args>(args)...);
  }

  template<typename T>
  ToPythonTaskFactory<T>::~ToPythonTaskFactory() {
    Python::GilRelease gil;
    boost::lock_guard<Python::GilRelease> lock{gil};
    m_factory.reset();
  }

  template<typename T>
  std::shared_ptr<Task> ToPythonTaskFactory<T>::Create() {
    return std::make_shared<ToPythonTask<Task>>(m_factory->Create());
  }

  template<typename T>
  boost::any& ToPythonTaskFactory<T>::FindProperty(const std::string& name) {
    return m_factory->FindProperty(name);
  }

  template<typename T>
  void ToPythonTaskFactory<T>::PrepareContinuation(const Task& task) {
    m_factory->PrepareContinuation(task);
  }
}
}

#endif
