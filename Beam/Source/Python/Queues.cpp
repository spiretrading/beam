#include "Beam/Python/Queues.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/Function.hpp"
#include "Beam/Python/ToPythonQueueWriter.hpp"
#include "Beam/Queues/AliasQueue.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Queues/TaskQueue.hpp"

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

  std::shared_ptr<QueueWriter<boost::python::object>> RoutineTaskQueueGetSlot(
      RoutineTaskQueue& queue,
      const NoThrowFunction<void, const boost::python::object&>& slot) {
    auto slotQueue = queue.GetSlot<SharedObject>(
      [=] (const SharedObject& p) {
        slot(*p);
      });
    auto taskQueue = MakeToPythonQueueWriter(
      std::static_pointer_cast<QueueWriter<SharedObject>>(slotQueue));
    return MakeAliasQueue(std::static_pointer_cast<
      QueueWriter<boost::python::object>>(taskQueue), slotQueue);
  }

  std::shared_ptr<QueueWriter<boost::python::object>>
      RoutineTaskQueueGetBreakSlot(RoutineTaskQueue& queue,
      const NoThrowFunction<void, const boost::python::object&>& slot,
      const NoThrowFunction<void, const std::exception_ptr&>& breakSlot) {
    auto slotQueue = queue.GetSlot<SharedObject>(
      [=] (const SharedObject& p) {
        slot(*p);
      }, breakSlot);
    auto taskQueue = MakeToPythonQueueWriter(
      std::static_pointer_cast<QueueWriter<SharedObject>>(slotQueue));
    return MakeAliasQueue(std::static_pointer_cast<
      QueueWriter<boost::python::object>>(taskQueue), slotQueue);
  }

  std::shared_ptr<QueueWriter<boost::python::object>> TaskQueueGetSlot(
      TaskQueue& queue,
      const NoThrowFunction<void, const boost::python::object&>& slot) {
    auto slotQueue = queue.GetSlot<SharedObject>(
      [=] (const SharedObject& p) {
        slot(*p);
      });
    auto taskQueue = MakeToPythonQueueWriter(
      std::static_pointer_cast<QueueWriter<SharedObject>>(slotQueue));
    return MakeAliasQueue(std::static_pointer_cast<
      QueueWriter<boost::python::object>>(taskQueue), slotQueue);
  }

  std::shared_ptr<QueueWriter<boost::python::object>>
      TaskQueueGetBreakSlot(TaskQueue& queue,
      const NoThrowFunction<void, const boost::python::object&>& slot,
      const NoThrowFunction<void, const std::exception_ptr&>& breakSlot) {
    auto slotQueue = queue.GetSlot<SharedObject>(
      [=] (const SharedObject& p) {
        slot(*p);
      }, breakSlot);
    auto taskQueue = MakeToPythonQueueWriter(
      std::static_pointer_cast<QueueWriter<SharedObject>>(slotQueue));
    return MakeAliasQueue(std::static_pointer_cast<
      QueueWriter<boost::python::object>>(taskQueue), slotQueue);
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(BaseQueue);
BEAM_DEFINE_PYTHON_QUEUE_LINKER(object);
BEAM_DEFINE_PYTHON_QUEUE_LINKER(std::function<void ()>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(RoutineTaskQueue);
BEAM_DEFINE_PYTHON_POINTER_LINKER(TaskQueue);
BEAM_DEFINE_PYTHON_POINTER_LINKER(FromPythonAbstractQueue<object>);

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

void Beam::Python::ExportQueues() {
  ExportBasePublisher();
  ExportBaseSnapshotPublisher();
  ExportBaseQueue();
  ExportQueueSuite<boost::python::object>("");
  ExportQueueSuite<std::function<void ()>>("VoidFunction");
  ExportRoutineTaskQueue();
  ExportTaskQueue();
  def("flush", &FlushPythonQueue);
  ExportException<PipeBrokenException, std::runtime_error>(
    "PipeBrokenException")
    .def(init<>())
    .def(init<const string&>());
}

void Beam::Python::ExportRoutineTaskQueue() {
  class_<RoutineTaskQueue, std::shared_ptr<RoutineTaskQueue>, noncopyable,
    bases<QueueWriter<std::function<void ()>>>>("RoutineTaskQueue", init<>())
    .def("get_slot", &RoutineTaskQueueGetSlot)
    .def("get_slot", &RoutineTaskQueueGetBreakSlot)
    .def("wait", BlockingFunction(&RoutineTaskQueue::Wait));
  implicitly_convertible<std::shared_ptr<RoutineTaskQueue>,
    std::shared_ptr<QueueWriter<std::function<void ()>>>>();
  implicitly_convertible<std::shared_ptr<RoutineTaskQueue>,
    std::shared_ptr<BaseQueue>>();
}

void Beam::Python::ExportTaskQueue() {
  class_<TaskQueue, std::shared_ptr<TaskQueue>, noncopyable,
    bases<AbstractQueue<std::function<void ()>>>>("TaskQueue", init<>())
    .def("get_slot", &TaskQueueGetSlot)
    .def("get_slot", &TaskQueueGetBreakSlot)
    .def("wait", BlockingFunction(&TaskQueue::Wait));
  implicitly_convertible<std::shared_ptr<TaskQueue>,
    std::shared_ptr<QueueReader<std::function<void ()>>>>();
  implicitly_convertible<std::shared_ptr<TaskQueue>,
    std::shared_ptr<QueueWriter<std::function<void ()>>>>();
  implicitly_convertible<std::shared_ptr<TaskQueue>,
    std::shared_ptr<AbstractQueue<std::function<void ()>>>>();
  implicitly_convertible<std::shared_ptr<TaskQueue>,
    std::shared_ptr<BaseQueue>>();
  def("handle_tasks", &HandleTasks);
}
