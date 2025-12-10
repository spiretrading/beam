#ifndef BEAM_PYTHON_QUEUES_HPP
#define BEAM_PYTHON_QUEUES_HPP
#include <string>
#include <pybind11/pybind11.h>
#include "Beam/Python/AbstractQueue.hpp"
#include "Beam/Python/QueueReader.hpp"
#include "Beam/Python/QueueWriter.hpp"
#include "Beam/Queues/Queue.hpp"
#include "Beam/Queues/QueueWriterPublisher.hpp"
#include "Beam/Queues/SnapshotPublisher.hpp"

namespace Beam::Python {

  /**
   * Exports the generic AbstractQueue class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void export_abstract_queue(
      pybind11::module& module, const std::string& prefix) {
    using Source = typename T::Source;
    using Target = typename T::Target;
    auto name = prefix + "AbstractQueue";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    auto binding = pybind11::class_<T, TrampolineAbstractQueue<T>,
      std::shared_ptr<T>, typename T::Reader, typename T::Writer>(
        module, name.c_str(), pybind11::multiple_inheritance());
    if constexpr(!std::is_same_v<typename T::Source, pybind11::object>) {
      binding.def(pybind11::init(
        [] (std::shared_ptr<AbstractQueue<pybind11::object>> queue) {
          return make_from_python_abstract_queue<Source>(std::move(queue));
        }));
      pybind11::implicitly_convertible<AbstractQueue<pybind11::object>, T>();
    }
  }

  /**
   * Exports the BasePublisher class.
   * @param module The module to export to.
   */
  void export_base_publisher(pybind11::module& module);

  /**
   * Exports the BaseQueue class.
   * @param module The module to export to.
   */
  void export_base_queue(pybind11::module& module);

  /**
   * Exports the generic Publisher class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void export_publisher(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Publisher";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<Publisher<T>, std::shared_ptr<Publisher<T>>,
      BasePublisher>(module, name.c_str(), pybind11::multiple_inheritance()).
      def("monitor", &Publisher<T>::monitor,
        pybind11::call_guard<pybind11::gil_scoped_release>());
  }

  /**
   * Exports the PublisherReactor.
   * @param module The module to export to.
   */
  void export_publisher_reactor(pybind11::module& module);

  /**
   * Exports the generic Queue class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void export_queue(pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "Queue";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<T, std::shared_ptr<T>, AbstractQueue<typename T::Target>>(
      module, name.c_str(), pybind11::multiple_inheritance()).
      def(pybind11::init()).
      def("pop", &T::pop, pybind11::call_guard<pybind11::gil_scoped_release>());
  }

  /**
   * Exports the QueueReactor.
   * @param module The module to export to.
   */
  void export_queue_reactor(pybind11::module& module);

  /**
   * Exports the generic QueueReader class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void export_queue_reader(
      pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "QueueReader";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    auto binding = pybind11::class_<
      T, TrampolineQueueReader<T>, std::shared_ptr<T>, BaseQueue>(
        module, name.c_str(), pybind11::multiple_inheritance()).
      def("pop", &T::pop).
      def("try_pop", &T::try_pop);
    if constexpr(!std::is_same_v<typename T::Source, pybind11::object>) {
      binding.def(pybind11::init(
        [] (std::shared_ptr<QueueReader<pybind11::object>> queue) {
          return make_from_python_queue_reader<typename T::Source>(
            std::move(queue));
        }));
      pybind11::implicitly_convertible<QueueReader<pybind11::object>, T>();
    }
  }

  /**
   * Exports the generic QueueWriter class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void export_queue_writer(
      pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "QueueWriter";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    auto binding = pybind11::class_<
      T, TrampolineQueueWriter<T>, std::shared_ptr<T>, BaseQueue>(
        module, name.c_str(), pybind11::multiple_inheritance()).
      def("push", pybind11::overload_cast<const typename T::Target&>(&T::push));
    if constexpr(!std::is_same_v<typename T::Target, pybind11::object>) {
      binding.def(pybind11::init(
        [] (std::shared_ptr<QueueWriter<pybind11::object>> queue) {
          return make_from_python_queue_writer<typename T::Target>(
            std::move(queue));
        }));
      pybind11::implicitly_convertible<QueueWriter<pybind11::object>, T>();
    }
  }

  /**
   * Exports the generic QueueWriterPublisher class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void export_queue_writer_publisher(
      pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "QueueWriterPublisher";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    pybind11::class_<T, std::shared_ptr<T>, QueueWriter<typename T::Target>,
      Publisher<typename T::Target>>(
        module, name.c_str(), pybind11::multiple_inheritance()).
      def(pybind11::init()).
      def("__len__", &T::get_size);
  }

  /**
   * Exports a suite of Queue and Publisher classes for a specific type.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T>
  void export_queue_suite(pybind11::module& module, const std::string& prefix) {
    export_queue_reader<QueueReader<T>>(module, prefix);
    export_queue_writer<QueueWriter<T>>(module, prefix);
    export_abstract_queue<AbstractQueue<T>>(module, prefix);
    export_queue<Queue<T>>(module, prefix);
    export_publisher<T>(module, prefix);
    export_queue_writer_publisher<QueueWriterPublisher<T>>(module, prefix);
  }

  /**
   * Exports all Queues and Publishers.
   * @param module The module to export to.
   */
  void export_queues(pybind11::module& module);

  /**
   * Exports the RoutineTaskQueue class.
   * @param module The module to export to.
   */
  void export_routine_task_queue(pybind11::module& module);

  /**
   * Exports the generic SnapshotPublisher class.
   * @param module The module to export to.
   * @param prefix The prefix used when forming the type name.
   */
  template<typename T, typename S>
  void export_snapshot_publisher(
      pybind11::module& module, const std::string& prefix) {
    auto name = prefix + "SnapshotPublisher";
    if(pybind11::hasattr(module, name.c_str())) {
      return;
    }
    export_publisher<T>(module, prefix);
    pybind11::class_<SnapshotPublisher<T, S>,
        std::shared_ptr<SnapshotPublisher<T, S>>, Publisher<T>>(
          module, name.c_str(), pybind11::multiple_inheritance()).
      def("snapshot_monitor",
        [] (SnapshotPublisher<T, S>& self, ScopedQueueWriter<T> writer) {
          auto snapshot = boost::optional<S>();
          {
            auto release = pybind11::gil_scoped_release();
            self.monitor(std::move(writer), out(snapshot));
          }
          if(snapshot) {
            return pybind11::cast(*snapshot);
          }
          return pybind11::object();
        }).
      def("get_snapshot", [] (SnapshotPublisher<T, S>& self) {
        auto snapshot = [&] {
          auto release = pybind11::gil_scoped_release();
          return self.get_snapshot();
        }();
        if(snapshot) {
          return pybind11::cast(*snapshot);
        }
        return pybind11::object();
      });
  }

  /**
   * Exports the TaskQueue class.
   * @param module The module to export to.
   */
  void export_task_queue(pybind11::module& module);
}

#endif
