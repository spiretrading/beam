#ifndef BEAM_PYTHON_JSON_HPP
#define BEAM_PYTHON_JSON_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports all JSON classes.
   * @param module The module to export to.
   */
  void ExportJson(pybind11::module& module);

  /**
   * Exports the JsonObject class.
   * @param module The module to export to.
   */
  void ExportJsonObject(pybind11::module& module);
}

#endif
