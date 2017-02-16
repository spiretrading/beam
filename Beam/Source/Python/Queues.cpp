#include "Beam/Python/Queues.hpp"
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/PythonQueue.hpp"
#include "Beam/Python/PythonRoutineTaskQueue.hpp"
#include "Beam/Python/PythonStateQueue.hpp"
#include "Beam/Python/PythonTaskQueue.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Queues/QueueWriter.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;
using namespace std;

namespace {
  void FlushPythonQueue(QueueReader<object>& queue, boost::python::list list) {
    try {
      while(true) {
        list.append(queue.Top());
        queue.Pop();
      }
    } catch(const std::exception&) {}
  }

  void HandlePythonTasks(PythonTaskQueue& tasks) {
    while(!tasks.IsEmpty()) {
      auto task = tasks.Top();
      tasks.Pop();
      task();
    }
  }
}

void Beam::Python::ExportAbstractQueue() {
  class_<AbstractQueue<object>, std::shared_ptr<AbstractQueue<object>>,
    noncopyable, bases<QueueWriter<object>, QueueReader<object>>>(
    "AbstractQueue", no_init);
  implicitly_convertible<std::shared_ptr<AbstractQueue<object>>,
    std::shared_ptr<QueueReader<object>>>();
  implicitly_convertible<std::shared_ptr<AbstractQueue<object>>,
    std::shared_ptr<QueueWriter<object>>>();
  implicitly_convertible<std::shared_ptr<AbstractQueue<object>>,
    std::shared_ptr<BaseQueue>>();
}

void Beam::Python::ExportBasePublisher() {
  class_<BasePublisher, noncopyable>("BasePublisher", no_init);
}

void Beam::Python::ExportBaseSnapshotPublisher() {
  class_<BaseSnapshotPublisher, noncopyable>("BaseSnapshotPublisher", no_init);
}

void Beam::Python::ExportBaseQueue() {
  class_<BaseQueue, std::shared_ptr<BaseQueue>, noncopyable>("BaseQueue",
    no_init)
    .def("close", static_cast<void (BaseQueue::*)()>(&BaseQueue::Break));
}

void Beam::Python::ExportPythonQueueWriter() {
  class_<PythonQueueWriter, std::shared_ptr<PythonQueueWriter>, noncopyable>(
    "PythonQueueWriter", no_init);
}

void Beam::Python::ExportQueue() {
  class_<PythonQueue, std::shared_ptr<PythonQueue>, noncopyable,
    bases<AbstractQueue<object>, PythonQueueWriter>>("Queue", init<>());
  implicitly_convertible<std::shared_ptr<PythonQueue>,
    std::shared_ptr<AbstractQueue<object>>>();
  implicitly_convertible<std::shared_ptr<PythonQueue>,
    std::shared_ptr<QueueReader<object>>>();
  implicitly_convertible<std::shared_ptr<PythonQueue>,
    std::shared_ptr<QueueWriter<object>>>();
  implicitly_convertible<std::shared_ptr<PythonQueue>,
    std::shared_ptr<BaseQueue>>();
  implicitly_convertible<std::shared_ptr<PythonQueue>,
    std::shared_ptr<PythonQueueWriter>>();
}

void Beam::Python::ExportQueueReader() {
  class_<QueueReader<object>, std::shared_ptr<QueueReader<object>>,
    noncopyable, bases<BaseQueue>>("QueueReader", no_init)
    .add_property("is_empty", &QueueReader<object>::IsEmpty)
    .def("top", BlockingFunction(&QueueReader<object>::Top))
    .def("pop", &QueueReader<object>::Pop);
  implicitly_convertible<std::shared_ptr<QueueReader<object>>,
    std::shared_ptr<BaseQueue>>();
  def("flush", BlockingFunction(&FlushPythonQueue));
}

void Beam::Python::ExportQueues() {
  ExportBasePublisher();
  ExportBaseSnapshotPublisher();
  ExportBaseQueue();
  ExportQueueReader();
  ExportQueueWriter();
  ExportAbstractQueue();
  ExportPythonQueueWriter();
  ExportQueue();
  ExportRoutineTaskQueue();
  ExportStateQueue();
  ExportTaskQueue();
}

void Beam::Python::ExportQueueWriter() {
  class_<QueueWriter<object>, std::shared_ptr<QueueWriter<object>>,
    noncopyable, bases<BaseQueue>>("QueueWriter", no_init)
    .def("push", static_cast<void (QueueWriter<object>::*)(const object&)>(
      &QueueWriter<object>::Push));
  implicitly_convertible<std::shared_ptr<QueueWriter<object>>,
    std::shared_ptr<BaseQueue>>();
}

void Beam::Python::ExportRoutineTaskQueue() {
  class_<PythonRoutineTaskQueue, std::shared_ptr<PythonRoutineTaskQueue>,
    noncopyable, bases<QueueWriter<object>>>("RoutineTaskQueue", init<>())
    .def("get_slot", &PythonRoutineTaskQueue::GetSlot);
  implicitly_convertible<std::shared_ptr<PythonRoutineTaskQueue>,
    std::shared_ptr<QueueWriter<object>>>();
  implicitly_convertible<std::shared_ptr<PythonRoutineTaskQueue>,
    std::shared_ptr<BaseQueue>>();
}

void Beam::Python::ExportStateQueue() {
  class_<PythonStateQueue, std::shared_ptr<PythonStateQueue>, noncopyable,
    bases<AbstractQueue<object>, PythonQueueWriter>>("StateQueue", init<>());
  implicitly_convertible<std::shared_ptr<PythonStateQueue>,
    std::shared_ptr<AbstractQueue<object>>>();
  implicitly_convertible<std::shared_ptr<PythonStateQueue>,
    std::shared_ptr<QueueReader<object>>>();
  implicitly_convertible<std::shared_ptr<PythonStateQueue>,
    std::shared_ptr<QueueWriter<object>>>();
  implicitly_convertible<std::shared_ptr<PythonStateQueue>,
    std::shared_ptr<BaseQueue>>();
  implicitly_convertible<std::shared_ptr<PythonStateQueue>,
    std::shared_ptr<PythonQueueWriter>>();
}

void Beam::Python::ExportTaskQueue() {
  class_<PythonTaskQueue, std::shared_ptr<PythonTaskQueue>, noncopyable,
    bases<QueueWriter<object>>>("TaskQueue", init<>())
    .def("get_slot", &PythonTaskQueue::GetSlot);
  implicitly_convertible<std::shared_ptr<PythonTaskQueue>,
    std::shared_ptr<QueueWriter<object>>>();
  implicitly_convertible<std::shared_ptr<PythonTaskQueue>,
    std::shared_ptr<BaseQueue>>();
  def("handle_tasks", BlockingFunction(&HandlePythonTasks));
}
