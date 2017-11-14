#ifndef BEAM_TASKS_HPP
#define BEAM_TASKS_HPP
#include "Beam/Pointers/ClonePtr.hpp"

namespace Beam {
namespace Tasks {
  class AggregateTask;
  class AggregateTaskFactory;
  class BasicTask;
  template<typename T> class BasicTaskFactory;
  class ChainedTask;
  class ChainedTaskFactory;
  class IdleTask;
  class IdleTaskFactory;
  template<typename PackageType> class PackagedTask;
  template<typename PackageType> class PackagedTaskFactory;
  class ReactorMonitorTask;
  class ReactorMonitorTaskFactory;
  class ReactorTask;
  class ReactorTaskFactory;
  class SpawnTask;
  class SpawnTaskFactory;
  class Task;
  class TaskPropertyNotFoundException;
  class TaskStateParser;
  class UntilTask;
  class UntilTaskFactory;
  class VirtualTaskFactory;
  class WhenTask;
  class WhenTaskFactory;
  using TaskFactory = ClonePtr<VirtualTaskFactory>;
}
}

#endif
