#ifndef BEAM_IDLE_TASK_HPP
#define BEAM_IDLE_TASK_HPP
#include "Beam/Tasks/BasicTask.hpp"
#include "Beam/Tasks/Tasks.hpp"

namespace Beam {
namespace Tasks {

  /*! \class IdleTask
      \brief A Task that performs no operation.
   */
  class IdleTask : public BasicTask {
    public:

      //! Constructs an IdleTask.
      IdleTask() = default;

    protected:
      virtual void OnExecute() override final;

      virtual void OnCancel() override final;
 };

  /*! \class IdleTaskFactory
      \brief Implements the TaskFactory for IdleTasks.
   */
  class IdleTaskFactory : public BasicTaskFactory<IdleTaskFactory> {
    public:
      virtual std::shared_ptr<Task> Create() override final;
  };

  inline void IdleTask::OnExecute() {
    SetActive();
  }

  inline void IdleTask::OnCancel() {
    SetTerminal(State::CANCELED);
  }

  inline std::shared_ptr<Task> IdleTaskFactory::Create() {
    return std::make_shared<IdleTask>();
  }
}
}

#endif
