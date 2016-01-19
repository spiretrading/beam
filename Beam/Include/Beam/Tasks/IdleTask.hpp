#ifndef BEAM_IDLETASK_HPP
#define BEAM_IDLETASK_HPP
#include "Beam/Tasks/BasicTask.hpp"

namespace Beam {
namespace Tasks {

  /*! \class IdleTask
      \brief A Task that performs no operation.
   */
  class IdleTask : public BasicTask {
    public:

      //! Constructs an IdleTask.
      IdleTask();

    protected:
      virtual void OnExecute();

      virtual void OnCancel();
 };

  /*! \class IdleTaskFactory
      \brief Implements the TaskFactory for IdleTasks.
   */
  class IdleTaskFactory : public BasicTaskFactory<IdleTaskFactory> {
    public:
      virtual std::shared_ptr<Task> Create();
  };

  inline IdleTask::IdleTask() {}

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
