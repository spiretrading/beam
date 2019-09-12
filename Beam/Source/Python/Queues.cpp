#include "Beam/Python/Queues.hpp"
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/QueueWriter.hpp"
#include "Beam/Queues/BaseQueue.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Queues/TaskQueue.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

void Beam::Python::ExportBasePublisher(pybind11::module& module) {
  class_<BasePublisher, std::shared_ptr<BasePublisher>>(module, "BasePublisher")
    .def("with", &BasePublisher::With, call_guard<GilRelease>());
}

void Beam::Python::ExportBaseQueue(pybind11::module& module) {
  class_<BaseQueue, std::shared_ptr<BaseQueue>>(module, "BaseQueue")
    .def("close", static_cast<void (BaseQueue::*)(const std::exception_ptr&)>(
      &BaseQueue::Break))
    .def("close", static_cast<void (BaseQueue::*)()>(&BaseQueue::Break));
}

void Beam::Python::ExportBaseSnapshotPublisher(pybind11::module& module) {
  class_<BaseSnapshotPublisher, std::shared_ptr<BaseSnapshotPublisher>>(module,
    "BaseSnapshotPublisher");
}

void Beam::Python::ExportQueues(pybind11::module& module) {
  ExportBasePublisher(module);
  ExportBaseQueue(module);
  ExportBaseSnapshotPublisher(module);
  ExportQueueSuite<object>(module, "");
  ExportQueueSuite<std::function<void ()>>(module, "VoidFunction");
  ExportRoutineTaskQueue(module);
  ExportTaskQueue(module);
  module.def("flush",
    [] (QueueReader<object>& queue, list l) {
      try {
        while(true) {
          l.append(queue.Top());
          queue.Pop();
        }
      } catch(const PipeBrokenException&) {}
    });
  register_exception<PipeBrokenException>(module, "PipeBrokenException");
}

void Beam::Python::ExportRoutineTaskQueue(pybind11::module& module) {
  class_<RoutineTaskQueue, std::shared_ptr<RoutineTaskQueue>,
    QueueWriter<std::function<void ()>>>(module, "RoutineTaskQueue")
    .def(init())
    .def("get_slot",
      [] (RoutineTaskQueue& self, std::function<void (const object&)> slot) {
        return MakeToPythonQueueWriter(
          std::static_pointer_cast<QueueWriter<object>>(
          self.GetSlot(std::move(slot))));
      })
    .def("get_slot",
      [] (RoutineTaskQueue& self,
          std::function<void (const object&)> slot,
          std::function<void (const std::exception_ptr&)> breakSlot) {
        return MakeToPythonQueueWriter(
          std::static_pointer_cast<QueueWriter<object>>(
          self.GetSlot(std::move(slot), std::move(breakSlot))));
      })
    .def("wait", &RoutineTaskQueue::Wait, call_guard<GilRelease>());
}

void Beam::Python::ExportTaskQueue(pybind11::module& module) {
  class_<TaskQueue, std::shared_ptr<TaskQueue>,
    AbstractQueue<std::function<void ()>>>(module, "TaskQueue")
    .def(init())
    .def("get_slot",
      [] (TaskQueue& self, std::function<void (const object&)> slot) {
        return MakeToPythonQueueWriter(
          std::static_pointer_cast<QueueWriter<object>>(
          self.GetSlot(std::move(slot))));
      })
    .def("get_slot",
      [] (TaskQueue& self, std::function<void (const object&)> slot,
          std::function<void (const std::exception_ptr&)> breakSlot) {
        return MakeToPythonQueueWriter(
          std::static_pointer_cast<QueueWriter<object>>(
          self.GetSlot(std::move(slot), std::move(breakSlot))));
      })
    .def("wait", &TaskQueue::Wait, call_guard<GilRelease>());
  module.def("handle_tasks", &HandleTasks);
}
