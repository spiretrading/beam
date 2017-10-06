#include "Beam/Python/Expect.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::python;

void Beam::Python::ExportExpect() {
  class_<Expect<object>>("Expect", init<>())
    .def(init<const object&>())
    .def("is_value", &Expect<object>::IsValue)
    .def("is_exception", &Expect<object>::IsException)
    .add_property("value", make_function(&Expect<object>::Get,
      return_value_policy<copy_const_reference>()));
}
