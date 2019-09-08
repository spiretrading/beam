#ifndef BEAM_PYTHON_QUEUES_HPP
#define BEAM_PYTHON_QUEUES_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Beam/Queues/AbstractQueue.hpp"
#include "Beam/Queues/Publisher.hpp"
#include "Beam/Queues/MultiQueueWriter.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueReader.hpp"
#include "Beam/Queues/QueueWriter.hpp"

namespace Beam::Python {
  template<typename T>
  struct FromPythonAbstractQueue final : T {
    using Target = typename T::Target;
    using Source = typename T::Source;
    using T::T;

    bool IsEmpty() const override {
      PYBIND11_OVERLOAD_PURE_NAME(bool, T, "is_empty", IsEmpty);
    }

    Target Top() const override {
      PYBIND11_OVERLOAD_PURE_NAME(Target, T, "top", Top);
    }

    void Pop() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, T, "pop", Pop);
    }

    void Push(Source&& value) override {
      Push(value);
    }

    void Push(const Source& value) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, T, "push", Push, value);
    }
  };

  template<typename T>
  struct FromPythonQueueReader final : T {
    using Target = typename T::Target;
    using T::T;

    bool IsEmpty() const override {
      PYBIND11_OVERLOAD_PURE_NAME(bool, T, "is_empty", IsEmpty);
    }

    Target Top() const override {
      PYBIND11_OVERLOAD_PURE_NAME(Target, T, "top", Top);
    }

    void Pop() override {
      PYBIND11_OVERLOAD_PURE_NAME(void, T, "pop", Pop);
    }
  };

  template<typename T>
  struct FromPythonQueueWriter final : T {
    using Source = typename T::Source;
    using T::T;

    void Push(Source&& value) override {
      Push(value);
    }

    void Push(const Source& value) override {
      PYBIND11_OVERLOAD_PURE_NAME(void, T, "push", Push, value);
    }
  };

  /**
   * Exports the BasePublisher class.
   * @param module The module to export to.
   */
  void ExportBasePublisher(pybind11::module& module);

  /**
   * Exports the BaseQueue class.
   * @param module The module to export to.
   */
  void ExportBaseQueue(pybind11::module& module);

  /**
   * Exports the BaseSnapshotPublisher class.
   * @param module The module to export to.
   */
  void ExportBaseSnapshotPublisher(pybind11::module& module);

  /**
   * Exports all Queues and Publishers.
   * @param module The module to export to.
   */
  void ExportQueues(pybind11::module& module);

  /**
   * Exports the RoutineTaskQueue class.
   * @param module The module to export to.
   */
  void ExportRoutineTaskQueue(pybind11::module& module);

  /**
   * Exports the TaskQueue class.
   * @param module The module to export to.
   */
  void ExportTaskQueue(pybind11::module& module);

  /**
   * Exports the generic AbstractQueue class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void ExportAbstractQueue(pybind11::module& module,
      const std::string& prefix) {
    using Target = typename T::Target;
    using Source = typename T::Source;
    auto name = prefix + "AbstractQueue";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<T, FromPythonAbstractQueue<T>, std::shared_ptr<T>,
      typename T::Reader, typename T::Writer>(module, name.c_str(),
      pybind11::multiple_inheritance());
  }

  /**
   * Exports the generic MultiQueueWriter class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void ExportMultiQueueWriter(pybind11::module& module,
      const std::string& prefix) {
    auto name = prefix + "MultiQueueWriter";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<T, std::shared_ptr<T>, QueueWriter<typename T::Source>,
        Publisher<typename T::Source>>(module, name.c_str(),
        pybind11::multiple_inheritance())
      .def(pybind11::init());
  }

  /**
   * Exports the generic Publisher class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void ExportPublisher(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Publisher";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<Publisher<T>, std::shared_ptr<Publisher<T>>,
        BasePublisher>(module, name.c_str(), pybind11::multiple_inheritance())
      .def("with", &Publisher<T>::With,
        pybind11::call_guard<pybind11::gil_scoped_release>())
      .def("monitor", &Publisher<T>::Monitor,
        pybind11::call_guard<pybind11::gil_scoped_release>());
  }

  /**
   * Exports the generic Queue class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void ExportQueue(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Queue";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<T, std::shared_ptr<T>, AbstractQueue<typename T::Source>>(
        module, name.c_str(), pybind11::multiple_inheritance())
      .def(pybind11::init())
      .def("is_broken", &T::IsBroken)
      .def("wait", static_cast<void (T::*)() const>(&T::Wait),
        pybind11::call_guard<pybind11::gil_scoped_release>())
      .def("top", &T::Top);
  }

  /**
   * Exports the generic QueueReader class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void ExportQueueReader(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "QueueReader";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<T, FromPythonQueueReader<T>, std::shared_ptr<T>,
        BaseQueue>(module, name.c_str(), pybind11::multiple_inheritance())
      .def("is_empty", &T::IsEmpty)
      .def("top", &T::Top)
      .def("pop", &T::Pop);
  }

  /**
   * Exports the generic QueueWriter class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void ExportQueueWriter(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "QueueWriter";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<T, FromPythonQueueWriter<T>, std::shared_ptr<T>,
        BaseQueue>(module, name.c_str(), pybind11::multiple_inheritance())
      .def("push", static_cast<void (T::*)(const typename T::Source&)>(
        &T::Push));
  }

  /**
   * Exports a suite of Queue and Publisher classes for a specific type.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void ExportQueueSuite(pybind11::module& module, const std::string& prefix) {
    ExportQueueReader<QueueReader<T>>(module, prefix);
    ExportQueueWriter<QueueWriter<T>>(module, prefix);
    ExportAbstractQueue<AbstractQueue<T>>(module, prefix);
    ExportQueue<Queue<T>>(module, prefix);
    ExportPublisher<T>(module, prefix);
    ExportMultiQueueWriter<MultiQueueWriter<T>>(module, prefix);
  }
}

#endif
