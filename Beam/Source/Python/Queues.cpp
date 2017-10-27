#include "Beam/Python/Queues.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/Exception.hpp"
#include "Beam/Python/PythonRoutineTaskQueue.hpp"
#include "Beam/Python/PythonTaskQueue.hpp"
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/PipeBrokenException.hpp"
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

#ifdef _MSC_VER
namespace boost {
  template<> inline const volatile AbstractQueue<object>* get_pointer(
      const volatile AbstractQueue<object>* p) {
    return p;
  }

  template<> inline const volatile Python::Details::AbstractQueueWrapper<
      AbstractQueue<object>>* get_pointer(
      const volatile Python::Details::AbstractQueueWrapper<
      AbstractQueue<object>>* p) {
    return p;
  }

  template<> inline const volatile BaseQueue* get_pointer(
      const volatile BaseQueue* p) {
    return p;
  }

  template<> inline const volatile PythonRoutineTaskQueue* get_pointer(
      const volatile PythonRoutineTaskQueue* p) {
    return p;
  }

  template<> inline const volatile PythonTaskQueue* get_pointer(
      const volatile PythonTaskQueue* p) {
    return p;
  }

  template<> inline const volatile Queue<object>* get_pointer(
      const volatile Queue<object>* p) {
    return p;
  }

  template<> inline const volatile QueueReader<object>* get_pointer(
      const volatile QueueReader<object>* p) {
    return p;
  }

  template<> inline const volatile Python::Details::QueueReaderWrapper<
      QueueReader<object>>* get_pointer(
      const volatile Python::Details::QueueReaderWrapper<
      QueueReader<object>>* p) {
    return p;
  }

  template<> inline const volatile QueueWriter<object>* get_pointer(
      const volatile QueueWriter<object>* p) {
    return p;
  }

  template<> inline const volatile Python::Details::QueueWriterWrapper<
      QueueWriter<object>>* get_pointer(
      const volatile Python::Details::QueueWriterWrapper<
      QueueWriter<object>>* p) {
    return p;
  }
}
#endif

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
  class_<PythonRoutineTaskQueue, std::shared_ptr<PythonRoutineTaskQueue>,
    noncopyable, bases<QueueWriter<object>>>("RoutineTaskQueue", init<>())
    .def("get_slot", &PythonRoutineTaskQueue::GetSlot)
    .def("wait", &PythonRoutineTaskQueue::Wait);
  implicitly_convertible<std::shared_ptr<PythonRoutineTaskQueue>,
    std::shared_ptr<QueueWriter<object>>>();
  implicitly_convertible<std::shared_ptr<PythonRoutineTaskQueue>,
    std::shared_ptr<BaseQueue>>();
}

void Beam::Python::ExportTaskQueue() {
  class_<PythonTaskQueue, std::shared_ptr<PythonTaskQueue>, noncopyable,
    bases<QueueWriter<object>>>("TaskQueue", init<>())
    .def("get_slot", &PythonTaskQueue::GetSlot);
  implicitly_convertible<std::shared_ptr<PythonTaskQueue>,
    std::shared_ptr<QueueWriter<object>>>();
  implicitly_convertible<std::shared_ptr<PythonTaskQueue>,
    std::shared_ptr<BaseQueue>>();
  def("handle_tasks", &HandlePythonTasks);
}
