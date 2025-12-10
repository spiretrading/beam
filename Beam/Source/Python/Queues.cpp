#include "Beam/Python/Queues.hpp"
#include <Aspen/Conversions.hpp>
#include <Aspen/Python/Box.hpp>
#include <Aspen/Python/Reactor.hpp>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonFunction.hpp"
#include "Beam/Python/QueueWriter.hpp"
#include "Beam/Queues/BaseQueue.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/PublisherReactor.hpp"
#include "Beam/Queues/QueueReactor.hpp"
#include "Beam/Queues/RoutineTaskQueue.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"
#include "Beam/Queues/TaskQueue.hpp"

using namespace Aspen;
using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

void Beam::Python::export_base_publisher(pybind11::module& module) {
  class_<BasePublisher, std::shared_ptr<BasePublisher>>(
    module, "BasePublisher").
    def("apply", static_cast<void (BasePublisher::*)(
      const std::function<void ()>&) const>(&BasePublisher::with),
      call_guard<GilRelease>());
}

void Beam::Python::export_base_queue(pybind11::module& module) {
  class_<BaseQueue, std::shared_ptr<BaseQueue>>(module, "BaseQueue").
    def("close", static_cast<void (BaseQueue::*)(const std::exception_ptr&)>(
      &BaseQueue::close)).
    def("close", static_cast<void (BaseQueue::*)()>(&BaseQueue::close));
}

void Beam::Python::export_publisher_reactor(pybind11::module& module) {
  module.def("monitor", [] (Publisher<object>& publisher) {
    return shared_box(publisher_reactor(std::move(publisher)));
  });
}

void Beam::Python::export_queue_reactor(pybind11::module& module) {
  export_reactor<QueueReactor<object>>(module, "QueueReactor").
    def(pybind11::init<std::shared_ptr<QueueReader<object>>>());
}

void Beam::Python::export_queues(pybind11::module& module) {
  export_base_publisher(module);
  export_base_queue(module);
  export_publisher_reactor(module);
  export_queue_suite<object>(module, "");
  export_queue_suite<std::function<void ()>>(module, "VoidFunction");
  export_routine_task_queue(module);
  export_task_queue(module);
  module.def("flush", [] (QueueReader<object>& queue, list l) {
    try {
      while(true) {
        auto value = [&] {
          auto release = GilRelease();
          return queue.pop();
        }();
        l.append(std::move(value));
      }
    } catch(const std::exception&) {}
  });
  register_exception<PipeBrokenException>(module, "PipeBrokenException");
}

void Beam::Python::export_routine_task_queue(pybind11::module& module) {
  class_<RoutineTaskQueue, std::shared_ptr<RoutineTaskQueue>,
    QueueWriter<std::function<void ()>>>(module, "RoutineTaskQueue").
    def(pybind11::init(&make_python_shared<RoutineTaskQueue>)).
    def("get_slot",
      [] (RoutineTaskQueue& self,
          const PythonFunction<void (const SharedObject&)>& slot) {
        return make_to_python_queue_writer(self.get_slot<SharedObject>(slot));
      }).
    def("get_slot",
      [] (RoutineTaskQueue& self,
          const PythonFunction<void (const SharedObject&)>& slot,
          const PythonFunction<void (const std::exception_ptr&)>& break_slot) {
        return make_to_python_queue_writer(
          self.get_slot<SharedObject>(slot, break_slot));
      }).
    def("wait", &RoutineTaskQueue::wait, call_guard<GilRelease>());
}

void Beam::Python::export_task_queue(pybind11::module& module) {
  class_<TaskQueue, std::shared_ptr<TaskQueue>,
    AbstractQueue<std::function<void ()>>>(module, "TaskQueue").
    def(pybind11::init()).
    def("get_slot",
      [] (TaskQueue& self, std::function<void (const object&)> slot) {
        auto queue = self.get_slot(std::move(slot));
        return make_to_python_queue_writer(std::move(queue));
      }).
    def("get_slot",
      [] (TaskQueue& self, std::function<void (const object&)> slot,
          std::function<void (const std::exception_ptr&)> break_slot) {
        auto queue = self.get_slot(std::move(slot), std::move(break_slot));
        return make_to_python_queue_writer(std::move(queue));
      }).
    def("pop", &TaskQueue::pop, call_guard<GilRelease>());
  module.def("flush", [] (TaskQueue& queue) {
    flush(queue);
  });
}
