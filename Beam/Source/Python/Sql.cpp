#include "Beam/Python/Sql.hpp"
#ifdef _MSC_VER
  #define _WINSOCKAPI_
#endif
#include "Beam/Python/BoostPython.hpp"
#include "Beam/Python/PythonBindings.hpp"
#include "Beam/Sql/PosixTimeToSqlDateTime.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::posix_time;
using namespace boost::python;
using namespace std;

void Beam::Python::ExportSql() {
  def("to_sql_timestamp", &ToSqlTimestamp);
  def("from_sql_timestamp", &FromSqlTimestamp);
}
