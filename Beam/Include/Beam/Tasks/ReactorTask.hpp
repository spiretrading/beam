#ifndef BEAM_REACTOR_TASK_HPP
#define BEAM_REACTOR_TASK_HPP
#include <algorithm>
#include <iterator>
#include <string>
#include <utility>
#include <vector>
#include <boost/optional/optional.hpp>
#include "Beam/Pointers/ClonePtr.hpp"
#include "Beam/Pointers/Ref.hpp"
#include "Beam/Reactors/MultiReactor.hpp"
#include "Beam/Reactors/NonRepeatingReactor.hpp"
#include "Beam/Reactors/ReactorMonitor.hpp"
#include "Beam/Reactors/Trigger.hpp"
#include "Beam/SignalHandling/ScopedSlotAdaptor.hpp"
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/Tasks.hpp"

namespace Beam {
namespace Tasks {

  /*! \class VirtualReactorProperty
      \brief Connects a single Reactor to a TaskFactory's property.
   */
  class VirtualReactorProperty : public virtual Cloneable {
    public:
      virtual ~VirtualReactorProperty() = default;

      //! Returns the name of the property.
      const std::string& GetName() const;

      //! Returns the Reactor to connect to the property.
      virtual std::shared_ptr<Reactors::BaseReactor> GetReactor() const = 0;

      //! Commits a change to the property.
      virtual void Commit() = 0;

      //! Applies this property to a TaskFactory.
      /*!
        \param factory The TaskFactory to apply this property to.
      */
      virtual void Apply(TaskFactory& factory) const = 0;

    protected:

      //! Constructs a VirtualReactorProperty.
      /*!
        \param name The name of the property.
      */
      VirtualReactorProperty(const std::string& name);

    private:
      std::string m_name;
  };

  typedef ClonePtr<VirtualReactorProperty> ReactorProperty;

  /*! \class ReactorTask
      \brief Executes a Task whose properties are connected to Reactors.
   */
  class ReactorTask : public BasicTask {
    public:

      //! Constructs a ReactorTask.
      /*!
        \param taskFactory Specifies the Task to connect the Reactors to.
        \param properties The properties to connect to the Task.
        \param reactorMonitor The ReactorMonitor to use.
      */
      ReactorTask(const TaskFactory& taskFactory,
        std::vector<ReactorProperty> properties,
        RefType<Reactors::ReactorMonitor> reactorMonitor);

    protected:
      virtual void OnExecute();

      virtual void OnCancel();

    private:
      TaskFactory m_taskFactory;
      std::vector<ReactorProperty> m_properties;
      Reactors::ReactorMonitor* m_reactorMonitor;
      Reactors::Trigger m_trigger;
      std::shared_ptr<Reactors::BaseReactor> m_propertyReactor;
      StateEntry m_propertyFailure;
      bool m_isPropertyUpdated;
      std::shared_ptr<Task> m_task;
      int m_state;
      SignalHandling::ScopedSlotAdaptor m_callbacks;

      bool OnPropertyUpdate(
        const std::vector<const Reactors::BaseReactor*>& updates);
      void OnTaskUpdate(const StateEntry& state);
      void S0();
      void S1();
      void S2();
      void S3();
      void S4();
      void S5(const StateEntry& state);
      void S6(const StateEntry& state);
      void S7();
  };

  /*! \class ReactorTaskFactory
      \brief Implements the TaskFactory for ReactorTasks.
   */
  class ReactorTaskFactory : public BasicTaskFactory<ReactorTaskFactory> {
    public:

      //! Constructs a ReactorTaskFactory.
      /*!
        \param taskFactory Specifies the Task to connect the Reactors to.
        \param properties The properties to connect to the Task.
        \param reactorMonitor The ReactorMonitor to use.
      */
      ReactorTaskFactory(const TaskFactory& taskFactory,
        std::vector<ReactorProperty> properties,
        RefType<Reactors::ReactorMonitor> reactorMonitor);

      virtual std::shared_ptr<Task> Create();

      virtual void PrepareContinuation(const Task& task);

    private:
      TaskFactory m_taskFactory;
      std::vector<ReactorProperty> m_properties;
      Reactors::ReactorMonitor* m_reactorMonitor;
  };

  /*! \class TypedReactorProperty
      \brief Implements a VirtualReactorProperty for a specific type of Reactor.
      \tparam T The type that the Reactor/property evaluates to.
   */
  template<typename T>
  class TypedReactorProperty : public VirtualReactorProperty,
      public CloneableMixin<TypedReactorProperty<T>> {
    public:

      //! Constructs a TypedReactorProperty.
      /*!
        \param name The name of the property.
        \param reactor The Reactor to connect to the property.
      */
      TypedReactorProperty(const std::string& name,
        const std::shared_ptr<Reactors::Reactor<T>>& reactor);

      //! Copies a TypedReactorProperty.
      /*!
        \param property The TypedReactorProperty to copy.
      */
      TypedReactorProperty(const TypedReactorProperty& property) = default;

      virtual std::shared_ptr<Reactors::BaseReactor> GetReactor() const;

      virtual void Commit();

      virtual void Apply(TaskFactory& factory) const;

    private:
      std::shared_ptr<Reactors::Reactor<T>> m_reactor;
      boost::optional<T> m_value;
  };

  //! Makes a ReactorProperty.
  /*!
    \param name The name of the property.
    \param reactor The Reactor to connect to the property.
  */
  template<typename T>
  TypedReactorProperty<T> MakeReactorProperty(const std::string& name,
      const std::shared_ptr<Reactors::Reactor<T>>& reactor) {
    return TypedReactorProperty<T>(name, reactor);
  }

  inline ReactorTask::ReactorTask(const TaskFactory& taskFactory,
      std::vector<ReactorProperty> properties,
      RefType<Reactors::ReactorMonitor> reactorMonitor)
      : m_taskFactory(taskFactory),
        m_properties(std::move(properties)),
        m_reactorMonitor(reactorMonitor.Get()),
        m_trigger(*m_reactorMonitor) {}

  inline void ReactorTask::OnExecute() {
    return S0();
  }

  inline void ReactorTask::OnCancel() {
    m_trigger.Do(
      [=] {
        if(m_state == 1) {
          return S6(StateEntry(State::CANCELED));
        } else if(m_state == 4) {
          return S6(StateEntry(State::CANCELED));
        } else if(m_state == 7) {
          return S6(StateEntry(State::CANCELED));
        }
      });
  }

  inline bool ReactorTask::OnPropertyUpdate(
      const std::vector<const Reactors::BaseReactor*>& updates) {
    m_isPropertyUpdated = true;
    for(const auto& property : m_properties) {
      try {
        property->Commit();
      } catch(const std::exception& e) {
        if(m_propertyFailure.m_state == State::NONE) {
          m_propertyFailure.m_state = State::FAILED;
          m_propertyFailure.m_message = e.what();
        }
      }
    }
    if(m_state == 1) {
      S1();
      return true;
    } else if(m_state == 4) {
      S4();
      return true;
    }
    return true;
  }

  inline void ReactorTask::OnTaskUpdate(const StateEntry& state) {
    if(m_state == 4) {
      if(state.m_state == State::FAILED) {
        return S5(state);
      } else if(IsTerminal(state.m_state)) {
        return S6(state);
      }
    } else if(m_state == 7) {
      if(state.m_state == State::FAILED) {
        return S5(state);
      } else if(state.m_state == State::CANCELED) {
        return S3();
      } else if(IsTerminal(state.m_state)) {
        return S6(state);
      }
    }
  }

  inline void ReactorTask::S0() {
    m_state = 0;
    m_isPropertyUpdated = false;
    std::vector<std::shared_ptr<Reactors::BaseReactor>> properties;
    std::transform(m_properties.begin(), m_properties.end(),
      std::back_inserter(properties),
      [] (const ReactorProperty& property) {
        return property->GetReactor();
      });
    m_propertyReactor = Reactors::MakeMultiReactor(m_callbacks.GetCallback(
      std::bind(&ReactorTask::OnPropertyUpdate, this, std::placeholders::_1),
      false), std::move(properties));
    S1();
    m_reactorMonitor->AddReactor(m_propertyReactor);
  }

  inline void ReactorTask::S1() {
    m_state = 1;
    if(m_propertyFailure.m_state != State::NONE) {

      // C0
      return S5(m_propertyFailure);
    } else if(m_isPropertyUpdated) {

      // C1
      return S2();
    }
  }

  inline void ReactorTask::S2() {
    m_state = 2;
    SetActive();
    return S3();
  }

  inline void ReactorTask::S3() {
    m_state = 3;
    m_isPropertyUpdated = false;
    if(m_task != nullptr) {
      m_taskFactory->PrepareContinuation(*m_task);
    }
    for(const auto& property : m_properties) {
      property->Apply(m_taskFactory);
    }
    m_task = m_taskFactory->Create();
    Manage(m_task);
    auto publisher = Reactors::MakePublisherReactor(&m_task->GetPublisher());
    auto taskReactor = Reactors::Do(m_callbacks.GetCallback(
      std::bind(&ReactorTask::OnTaskUpdate, this, std::placeholders::_1)),
      publisher);
    m_reactorMonitor->AddEvent(publisher);
    m_reactorMonitor->AddReactor(taskReactor);
    m_task->Execute();
    return S4();
  }

  inline void ReactorTask::S4() {
    m_state = 4;
    if(m_propertyFailure.m_state != State::NONE) {

      // C0
      return S5(m_propertyFailure);
    } else if(m_isPropertyUpdated) {

      // C1
      return S7();
    }
  }

  inline void ReactorTask::S5(const StateEntry& state) {
    m_state = 5;
    SetTerminal(state.m_state, state.m_message);
  }

  inline void ReactorTask::S6(const StateEntry& state) {
    m_state = 6;
    SetTerminal(state.m_state, state.m_message);
  }

  inline void ReactorTask::S7() {
    m_state = 7;
    m_task->Cancel();
  }

  inline ReactorTaskFactory::ReactorTaskFactory(const TaskFactory& taskFactory,
      std::vector<ReactorProperty> properties,
      RefType<Reactors::ReactorMonitor> reactorMonitor)
      : m_taskFactory(taskFactory),
        m_properties(std::move(properties)),
        m_reactorMonitor(reactorMonitor.Get()) {}

  inline std::shared_ptr<Task> ReactorTaskFactory::Create() {
    return std::make_shared<ReactorTask>(m_taskFactory, m_properties,
      Ref(*m_reactorMonitor));
  }

  inline void ReactorTaskFactory::PrepareContinuation(const Task& task) {}

  inline VirtualReactorProperty::VirtualReactorProperty(const std::string& name)
      : m_name(name) {}

  inline const std::string& VirtualReactorProperty::GetName() const {
    return m_name;
  }

  template<typename T>
  TypedReactorProperty<T>::TypedReactorProperty(const std::string& name,
      const std::shared_ptr<Reactors::Reactor<T>>& reactor)
      : VirtualReactorProperty(name),
        m_reactor(Reactors::MakeNonRepeatingReactor(reactor)) {}

  template<typename T>
  std::shared_ptr<Reactors::BaseReactor> TypedReactorProperty<T>::
      GetReactor() const {
    return m_reactor;
  }

  template<typename T>
  void TypedReactorProperty<T>::Commit() {
    m_value = m_reactor->Eval();
  }

  template<typename T>
  void TypedReactorProperty<T>::Apply(TaskFactory& factory) const {
    try {
      factory->Set(GetName(), *m_value);
    } catch(const TaskPropertyNotFoundException&) {}
  }
}
}

#endif
