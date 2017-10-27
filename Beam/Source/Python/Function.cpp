#include "Beam/Python/Function.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;

void Beam::Python::ExportFunctions() {
  ExportFunction<std::function<void ()>>("VoidFunction");
  ExportFunction<std::function<void (const object&)>>("VoidFunctionP1");
}
