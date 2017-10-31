#include "Beam/Python/Function.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;

void Beam::Python::ExportFunctions() {
  ExportFunction<std::function<void ()>>("VoidFunction");
  ExportFunction<std::function<void (const boost::python::object&)>>(
    "VoidFunctionP1");
  ExportFunction<std::function<void (const std::exception_ptr)>>(
    "VoidFunctionExceptionPtr");
  ExportFunction<NoThrowFunction<void>>("VoidNoThrowFunction");
  ExportFunction<NoThrowFunction<void, const boost::python::object&>>(
    "VoidNoThrowFunctionP1");
  ExportFunction<NoThrowFunction<void, const std::exception_ptr&>>(
    "VoidNoThrowFunctionExceptionPtr");
}
