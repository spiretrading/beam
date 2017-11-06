#ifndef BEAM_TO_PYTHON_TASK_HPP
#define BEAM_TO_PYTHON_TASK_HPP
#include <boost/optional/optional.hpp>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Tasks/Tasks.hpp"
#include "Beam/Tasks/Task.hpp"

namespace Beam {
namespace Tasks {
namespace Details {
  BEAM_EXPORT_DLL void AddTaskFactoryProperty(const char* name, void* property);
}

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

      //! Returns the wrapped TaskFactory.
      const WrappedTaskFactory& GetFactory() const;

      //! Returns the wrapped TaskFactory.
      WrappedTaskFactory& GetFactory();

      virtual std::shared_ptr<Task> Create();

      virtual boost::any& FindProperty(const std::string& name);

      virtual void PrepareContinuation(const Task& task);

    private:
      boost::optional<WrappedTaskFactory> m_factory;
  };

  /*! \class BasePythonTaskFactoryProperty
      \brief Base class used to set a generic TaskFactory's property.
   */
  class BasePythonTaskFactoryProperty {
    public:

      //! Gets a TaskFactory's property.
      /*!
        \param property The object storing the property.
        \return The Python object stored within the <i>property</i>.
      */
      virtual boost::python::object Get(const boost::any& property) const = 0;

      //! Sets a TaskFactory's property.
      /*!
        \param factory The TaskFactory to update.
        \param name The name of the property.
        \param value The property's value.
      */
      virtual bool Set(VirtualTaskFactory& factory, const std::string& name,
        const boost::python::object& value) const = 0;
  };

  /*! \class PythonTaskFactoryProperty
      \brief Implements the BasePythonTaskFactoryProperty for a specified type.
      \tparam T The type to implement the BasePythonTaskFactoryProperty for.
   */
  template<typename T>
  class PythonTaskFactoryProperty : public BasePythonTaskFactoryProperty {
    public:
      virtual boost::python::object Get(
        const boost::any& property) const override final;

      virtual bool Set(VirtualTaskFactory& factory, const std::string& name,
        const boost::python::object& value) const override final;
  };

  //! Exports a TaskFactory property of a specified type.
  template<typename T>
  void ExportTaskFactoryProperty() {
    auto property = new PythonTaskFactoryProperty<T>{};
    Details::AddTaskFactoryProperty(typeid(T).name(), property);
  }

  //! Gets a TaskFactory's property.
  /*!
    \param factory The TaskFactory to update.
    \param name The name of the property to update.
    \return The property with the specified <i>name</i>.
  */
  BEAM_EXPORT_DLL boost::python::object GetTaskFactoryProperty(
    VirtualTaskFactory* factory, const char* name);

  //! Sets a TaskFactory's property.
  /*!
    \param factory The TaskFactory to update.
    \param name The name of the property to update.
    \param value The value to assign to the property.
  */
  BEAM_EXPORT_DLL void SetTaskFactoryProperty(VirtualTaskFactory* factory,
    const char* name, const boost::python::object* value);

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
  const typename ToPythonTaskFactory<T>::WrappedTaskFactory&
      ToPythonTaskFactory<T>::GetFactory() const {
    return *m_factory;
  }

  template<typename T>
  typename ToPythonTaskFactory<T>::WrappedTaskFactory&
      ToPythonTaskFactory<T>::GetFactory() {
    return *m_factory;
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
    auto& pythonTask = static_cast<const ToPythonTask<Task>&>(task);
    m_factory->PrepareContinuation(pythonTask.GetTask());
  }

  template<typename T>
  boost::python::object PythonTaskFactoryProperty<T>::Get(
      const boost::any& property) const {
    return boost::python::object{boost::any_cast<T>(property)};
  }

  template<typename T>
  bool PythonTaskFactoryProperty<T>::Set(VirtualTaskFactory& factory,
      const std::string& name, const boost::python::object& value) const {
    boost::python::extract<T> extractor{value};
    if(extractor.check()) {
      factory.Set(name, extractor());
      return true;
    }
    return false;
  }
}
}

#endif
