#include "Beam/Python/Routines.hpp"
#include <boost/throw_exception.hpp>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include "Beam/Python/GilRelease.hpp"
#include "Beam/Python/PythonFunction.hpp"
#include "Beam/Python/SharedObject.hpp"
#include "Beam/Routines/Routine.hpp"
#include "Beam/Routines/RoutineException.hpp"
#include "Beam/Routines/RoutineHandler.hpp"
#include "Beam/Routines/RoutineHandlerGroup.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

void Beam::Python::export_base_async(module& module) {
  auto outer = class_<BaseAsync>(module, "BaseAsync");
  enum_<BaseAsync::State>(outer, "State").
    value("PENDING", BaseAsync::State::PENDING).
    value("COMPLETE", BaseAsync::State::COMPLETE).
    value("EXCEPTION", BaseAsync::State::EXCEPTION);
}

void Beam::Python::export_base_eval(module& module) {
  class_<BaseEval>(module, "BaseEval").
    def("set_exception", [] (BaseEval& self, object exception) {
      try {
        exception.inc_ref();
        boost::throw_with_location(error_already_set());
      } catch (...) {
        self.set_exception(std::current_exception());
      }
    });
}

template<>
void Beam::Python::export_eval<void>(
    module& module, const std::string& prefix) {
  auto name = prefix + "Eval";
  if(hasattr(module, name.c_str())) {
    return;
  }
  class_<Eval<void>, BaseEval>(module, name.c_str()).
    def(init()).
    def_property_readonly("is_empty", &Eval<void>::is_empty).
    def("set", &Eval<void>::set);
}

void Beam::Python::export_routine_handler(module& module) {
  class_<RoutineHandler, std::shared_ptr<RoutineHandler>>(
      module, "RoutineHandler").
    def(pybind11::init(&make_python_shared<RoutineHandler>)).
    def(pybind11::init(&make_python_shared<RoutineHandler, Routine::Id>)).
    def_property_readonly("id", &RoutineHandler::get_id).
    def("detach", &RoutineHandler::detach).
    def("wait", &RoutineHandler::wait, call_guard<GilRelease>());
  module.def("flush_pending_routines", &flush_pending_routines,
    call_guard<GilRelease>());
}

void Beam::Python::export_routine_handler_group(module& module) {
  class_<RoutineHandlerGroup, std::shared_ptr<RoutineHandlerGroup>>(
      module, "RoutineHandlerGroup").
    def(pybind11::init(&make_python_shared<RoutineHandlerGroup>)).
    def("add", overload_cast<Routine::Id>(&RoutineHandlerGroup::add)).
    def("add", [] (RoutineHandlerGroup& self, RoutineHandler* handler) {
      self.add(std::move(*handler));
    }).
    def("spawn",
      [] (RoutineHandlerGroup& self, const PythonFunction<void ()>& callable) {
        return self.spawn(callable);
      }).
    def("wait", &RoutineHandlerGroup::wait, call_guard<GilRelease>());
}

void Beam::Python::export_routines(module& module) {
  export_base_async(module);
  export_base_eval(module);
  export_routine_handler(module);
  export_routine_handler_group(module);
  export_async<object>(module, "");
  export_eval<object>(module, "");
  module.def("spawn", [] (const PythonFunction<void ()>& callable) {
    return spawn(callable);
  });
  module.def("defer", &defer, call_guard<GilRelease>());
  module.def("wait", &wait, call_guard<GilRelease>());
  register_exception<RoutineException>(module, "RoutineException");
}
