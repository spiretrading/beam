#include "Beam/Python/MySql.hpp"
#ifdef _MSC_VER
  #define _WINSOCKAPI_
#endif
#include "Beam/MySql/PosixTimeToMySqlDateTime.hpp"
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/PythonBindings.hpp"

using namespace Beam;
using namespace Beam::MySql;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

void Beam::Python::ExportMySql() {
  string nestedName = extract<string>(scope().attr("__name__") + ".mysql");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("mysql") = nestedModule;
  scope parent = nestedModule;
  def("to_mysql_timestamp", &ToMySqlTimestamp);
  def("from_mysql_timestamp", &FromMySqlTimestamp);
}
