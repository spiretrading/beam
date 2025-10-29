#include "Beam/Python/Sql.hpp"
#include <pybind11/pybind11.h>
#include "Beam/Sql/PosixTimeToSqlDateTime.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

void Beam::Python::export_sql(pybind11::module& module) {
  module.def("to_sql_timestamp", &to_sql_timestamp);
  module.def("from_sql_timestamp", &from_sql_timestamp);
}
