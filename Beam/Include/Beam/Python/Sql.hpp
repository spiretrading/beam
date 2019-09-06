#ifndef BEAM_PYTHON_SQL_HPP
#define BEAM_PYTHON_SQL_HPP
#include <pybind11/pybind11.h>

namespace Beam::Python {

  /**
   * Exports the Sql classes/functions.
   * @param module The module to export to.
   */
  void ExportSql(pybind11::module& module);
}

#endif
