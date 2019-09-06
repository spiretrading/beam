#ifndef BEAM_PYTHON_YAML_HPP
#define BEAM_PYTHON_YAML_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the Yaml class.
   */
  void ExportYaml(pybind11::module& module);
}

#endif
