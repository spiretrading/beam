#ifndef BEAM_TASK_HPP
#define BEAM_TASK_HPP
#include <memory>
#include <string>
#include <vector>
#include <boost/any.hpp>
#include <boost/atomic/atomic.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>
#include <boost/throw_exception.hpp>
#include "Beam/Collections/Enum.hpp"
#include "Beam/Pointers/ClonePtr.hpp"
#include "Beam/Queues/Publisher.hpp"
#include <Beam/Queues/Queue.hpp>
#include "Beam/Tasks/Tasks.hpp"
#include "Beam/Utilities/NotSupportedException.hpp"

namespace Beam {
namespace Tasks {
namespace Details {
  template<typename T>
  struct TaskDetails {
    static boost::atomic_llong m_nextId;
  };

  template<typename T>
  boost::atomic_llong TaskDetails<T>::m_nextId(0);

  BEAM_ENUM(TaskState,

    //! The Task is initializing, no sub-Tasks may be executed.
    INITIALIZING,

    //! Task is active, sub-Tasks may be executed.
    ACTIVE,

    //! Task is pending cancel, no new sub-Tasks may be executed.
    PENDING_CANCEL,

    //! Task is canceled.
    CANCELED,

    //! Task encountered an error.
    FAILED,

    //! The Task has expired.
    EXPIRED,

    //! Task has completed.
    COMPLETE);
}

  /*! \class Task
      \brief Executes and manages an asynchronous operation and sub-Tasks.
   */
  class Task {
    public:

      /*! \enum State
          \brief Enumerates Task states.
       */
      using State = Details::TaskState;

      /*! \struct StateEntry
          \brief Records a change in a Task's State.
       */
      struct StateEntry {

        //! The State of the Task.
        State m_state;

        //! A message describing the change in State.
        std::string m_message;

        //! Constructs an uninitialized StateEntry.
        StateEntry() = default;

        //! Constructs a StateEntry with an empty message.
        /*!
          \param state The Task's State.
        */
        StateEntry(State state);

        //! Constructs a StateEntry with an empty message.
        /*!
          \param state The Task's State.
        */
        StateEntry(State::Type state);

        //! Constructs a StateEntry.
        /*!
          \param state The Task's State.
          \param message A message describing the change in State.
        */
        StateEntry(State state, const std::string& message);
      };

      virtual ~Task() = default;

      //! Returns a unique id for this Task.
      long long GetId() const;

      //! Executes this Task.
      virtual void Execute() = 0;

      //! Cancels this Task.
      virtual void Cancel() = 0;

      //! Returns the object publishing StateEntry updates.
      /*!
        \param snapshot The current snapshot of all state transitions.
      */
      virtual const Publisher<StateEntry>& GetPublisher() const = 0;

    protected:

      //! Constructs a Task.
      Task();

    private:
      long long m_id;
  };

  /*! \class VirtualTaskFactory
      \brief Base class for a factory that creates Tasks.
   */
  class VirtualTaskFactory : public virtual Cloneable {
    public:
      virtual ~VirtualTaskFactory() = default;

      //! Creates a Task.
      virtual std::shared_ptr<Task> Create() = 0;

      //! Finds a property with a specified name.
      /*!
        \param name The name of the property to find.
        \return The property with the specified name.
      */
      const boost::any& FindProperty(const std::string& name) const;

      //! Finds a property with a specified name.
      /*!
        \param name The name of the property to find.
        \return The property with the specified name.
      */
      virtual boost::any& FindProperty(const std::string& name) = 0;

      //! Prepares a Task that's a continuation of another Task.
      /*!
        \param task The Task to prepare the continuation from.
      */
      virtual void PrepareContinuation(const Task& task);

      //! Sets a property value.
      /*!
        \param name The name of the property.
        \param value The value to set the property to.
      */
      template<typename T>
      void Set(const std::string& name, const T& value);

      //! Returns a property value.
      /*!
        \param name The name of the property.
        \return The value associated with the <i>name</i>.
      */
      template<typename T>
      const T& Get(const std::string& name) const;

      //! Returns a property value.
      /*!
        \param name The name of the property.
        \return The value associated with the <i>name</i>.
      */
      template<typename T>
      T& Get(const std::string& name);

    protected:

      //! Constructs a TaskFactory.
      VirtualTaskFactory() = default;

      //! Copies a TaskFactory.
      VirtualTaskFactory(const VirtualTaskFactory& factory) = default;
  };

  //! Returns the string representation of a Task's State.
  inline std::string ToString(Task::State state) {
    if(state == Task::State::NONE) {
      return "None";
    } else if(state == Task::State::INITIALIZING) {
      return "Initializing";
    } else if(state == Task::State::ACTIVE) {
      return "Active";
    } else if(state == Task::State::PENDING_CANCEL) {
      return "Pending Cancel";
    } else if(state == Task::State::CANCELED) {
      return "Canceled";
    } else if(state == Task::State::FAILED) {
      return "Failed";
    } else if(state == Task::State::EXPIRED) {
      return "Expired";
    } else if(state == Task::State::COMPLETE) {
      return "Complete";
    }
    BOOST_THROW_EXCEPTION(std::runtime_error{"Task::State not found: " +
      boost::lexical_cast<std::string>(static_cast<int>(state))});
  }

  //! Tests if a Task's State represents a terminal state.
  //! A terminal state is one of CANCELED, FAILED, EXPIRED, or COMPLETE.
  inline bool IsTerminal(Task::State state) {
    return state == Task::State::CANCELED ||
      state == Task::State::FAILED ||
      state == Task::State::EXPIRED ||
      state == Task::State::COMPLETE;
  }

  //! Blocks until a Task enters a terminal state.
  /*!
    \param task The Task to wait for.
  */
  inline void Wait(const Task& task) {
    auto queue = std::make_shared<Queue<Task::StateEntry>>();
    task.GetPublisher().Monitor(queue);
    try {
      while(true) {
        auto entry = queue->Top();
        queue->Pop();
        if(IsTerminal(entry.m_state)) {
          break;
        }
      }
    } catch(const std::exception&) {}
  }

  inline Task::StateEntry::StateEntry(State state)
      : m_state{state} {}

  inline Task::StateEntry::StateEntry(State::Type state)
      : m_state{state} {}

  inline Task::StateEntry::StateEntry(State state,
      const std::string& message)
      : m_state{state},
        m_message{message} {}

  inline long long Task::GetId() const {
    return m_id;
  }

  inline Task::Task()
      : m_id{++Details::TaskDetails<void>::m_nextId} {}

  inline const boost::any& VirtualTaskFactory::FindProperty(
      const std::string& name) const {
    return const_cast<VirtualTaskFactory*>(this)->FindProperty(name);
  }

  inline void VirtualTaskFactory::PrepareContinuation(const Task& task) {
    BOOST_THROW_EXCEPTION(NotSupportedException{"Task::PrepareContinuation"});
  }

  template<typename T>
  void VirtualTaskFactory::Set(const std::string& name, const T& value) {
    auto& property = FindProperty(name);
    *boost::any_cast<T>(&property) = value;
  }

  template<typename T>
  const T& VirtualTaskFactory::Get(const std::string& name) const {
    return *boost::any_cast<T>(&FindProperty(name));
  }

  template<typename T>
  T& VirtualTaskFactory::Get(const std::string& name) {
    return *boost::any_cast<T>(&FindProperty(name));
  }
}

  template<>
  struct EnumeratorCount<Tasks::Task::State> :
    std::integral_constant<int, 7> {};
}

#endif
