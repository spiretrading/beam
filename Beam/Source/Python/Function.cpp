#include "Beam/Python/Function.hpp"

using namespace Beam;
using namespace Beam::Python;

void Beam::Python::ExportFunctions() {
  ExportFunction<std::function<void ()>>("VoidFunction");
}
