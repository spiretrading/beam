#include "Beam/Python/SignalsSlots.hpp"
#include <boost/signals2/connection.hpp>

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;
using namespace boost::signals2;

void Beam::Python::ExportBoostConnection() {
  class_<connection>("connection", init<>())
    .def("disconnect", &connection::disconnect)
    .def("connected", &connection::connected);
}

void Beam::Python::ExportBoostScopedConnection() {
  class_<scoped_connection, boost::noncopyable>(
    "scoped_connection", init<const connection&>());
}

void Beam::Python::ExportSignalsSlots() {
  ExportBoostConnection();
  ExportBoostScopedConnection();
}
