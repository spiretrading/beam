#ifndef BEAM_PYTHON_THREADING_HPP
#define BEAM_PYTHON_THREADING_HPP
#include <string_view>
#include <type_traits>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Threading/Sync.hpp"

namespace Beam::Python {

  /**
   * Exports the ConditionVariable class.
   * @param module The module to export to.
   */
  void export_condition_variable(pybind11::module& module);

  /**
   * Exports the Mutex class.
   * @param module The module to export to.
   */
  void export_mutex(pybind11::module& module);

  /**
   * Exports the RecursiveMutex class.
   * @param module The module to export to.
   */
  void export_recursive_mutex(pybind11::module& module);

  /**
   * Exports the Sync template.
   * @param module The module to export to.
   * @param name The name of the class to export.
   */
  template<typename T>
  void export_sync(pybind11::module& module, std::string_view name) {
    pybind11::class_<Sync<T>>(module, name.data()).
      def(pybind11::init()).
      def(pybind11::init<const T&>()).
      def("load", &Sync<T>::load).
      def("apply",
        [] (Sync<T>& self, const std::function<pybind11::object (T&)>& f) {
          return self.with(f);
        }, pybind11::call_guard<GilRelease>());
    module.def("apply",
      [] (Sync<T>& sync, const std::function<pybind11::object (T&)>& f) {
        return with(sync, f);
      }, pybind11::call_guard<GilRelease>());
    module.def("apply",
      [] (const Sync<T>& sync,
          const std::function<pybind11::object (const T&)>& f) {
        return with(sync, f);
      }, pybind11::call_guard<GilRelease>());
  }

  /**
   * Exports the TaskRunner class.
   * @param module The module to export to.
   */
  void export_task_runner(pybind11::module& module);

  /**
   * Exports the ThreadPool class.
   * @param module The module to export to.
   */
  void export_thread_pool(pybind11::module& module);

  /**
   * Exports the TimedConditionVariable class.
   * @param module The module to export to.
   */
  void export_timed_condition_variable(pybind11::module& module);

  /**
   * Exports the Threading namespace.
   * @param module The module to export to.
   */
  void export_threading(pybind11::module& module);
}

#endif
