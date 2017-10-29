#include "Beam/Python/Queues.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Exception.hpp"
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
/*
  void HandlePythonTasks(PythonTaskQueue& tasks) {
    while(!tasks.IsEmpty()) {
      auto task = tasks.Top();
      tasks.Pop();
      task();
    }
  }
*/

  std::shared_ptr<QueueWriter<boost::python::object>> RoutineTaskQueueGetSlot(
      RoutineTaskQueue& queue,
      const std::function<void (const boost::python::object&)>& slot) {
    return queue.GetSlot<boost::python::object>(slot);
  }

  std::shared_ptr<QueueWriter<boost::python::object>>
      RoutineTaskQueueGetBreakSlot(RoutineTaskQueue& queue,
      const std::function<void (const boost::python::object&)>& slot,
      const std::function<void (const std::exception_ptr&)>& breakSlot) {
    return queue.GetSlot<boost::python::object>(slot, breakSlot);
  }
}

BEAM_DEFINE_PYTHON_POINTER_LINKER(AbstractQueue<object>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  Python::Details::AbstractQueueWrapper<AbstractQueue<object>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(BaseQueue);
BEAM_DEFINE_PYTHON_POINTER_LINKER(Queue<object>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(QueueReader<object>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  Python::Details::QueueReaderWrapper<QueueReader<object>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(QueueWriter<std::function<void ()>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  Python::Details::QueueWriterWrapper<QueueWriter<std::function<void ()>>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(QueueWriter<object>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(
  Python::Details::QueueWriterWrapper<QueueWriter<object>>);
BEAM_DEFINE_PYTHON_POINTER_LINKER(RoutineTaskQueue);

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
  ExportQueueReader<QueueReader<boost::python::object>>("QueueReader");
  ExportQueueWriter<QueueWriter<boost::python::object>>("QueueWriter");
  ExportAbstractQueue<AbstractQueue<boost::python::object>>("AbstractQueue");
  ExportQueue<Queue<boost::python::object>>("Queue");
  ExportRoutineTaskQueue();
  ExportTaskQueue();
  def("flush", &FlushPythonQueue);
  ExportException<PipeBrokenException, std::runtime_error>(
    "PipeBrokenException")
    .def(init<>())
    .def(init<const string&>());
}

void Beam::Python::ExportRoutineTaskQueue() {
  ExportQueueWriter<QueueWriter<std::function<void ()>>>("FunctionQueueWriter");
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
/*
  class_<PythonTaskQueue, std::shared_ptr<PythonTaskQueue>, noncopyable,
    bases<QueueWriter<object>>>("TaskQueue", init<>())
    .def("get_slot", &PythonTaskQueue::GetSlot);
  implicitly_convertible<std::shared_ptr<PythonTaskQueue>,
    std::shared_ptr<QueueWriter<object>>>();
  implicitly_convertible<std::shared_ptr<PythonTaskQueue>,
    std::shared_ptr<BaseQueue>>();
  def("handle_tasks", &HandlePythonTasks);
*/
}
