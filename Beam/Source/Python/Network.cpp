#include "Beam/Python/Network.hpp"
#include "Beam/Network/IpAddress.hpp"
#include "Beam/Python/BoostPython.hpp"

using namespace Beam;
using namespace Beam::Network;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;
using namespace std;

void Beam::Python::ExportIpAddress() {
  class_<IpAddress>("IpAddress", init<>())
    .def(init<std::string, unsigned short>())
    .add_property("host", make_function(&IpAddress::GetHost,
      return_value_policy<copy_const_reference>()))
    .add_property("port", &IpAddress::GetPort);
}

void Beam::Python::ExportNetwork() {
  string nestedName = extract<string>(scope().attr("__name__") +
    ".network");
  object nestedModule{handle<>(
    borrowed(PyImport_AddModule(nestedName.c_str())))};
  scope().attr("network") = nestedModule;
  scope parent = nestedModule;
  ExportIpAddress();
}
