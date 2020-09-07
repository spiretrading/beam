#include "Beam/Python/SharedObject.hpp"
#include "Beam/Python/GilLock.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

namespace {
  struct ObjectDeleter {
    void operator ()(pybind11::object* object) const {
      auto lock = GilLock();
      delete object;
    }
  };
}

SharedObject::SharedObject(pybind11::object object)
  : m_object(new pybind11::object(std::move(object)), ObjectDeleter()) {}

pybind11::object& SharedObject::operator *() const {
  return *m_object;
}

pybind11::object* SharedObject::operator ->() const {
  return m_object.get();
}

pybind11::handle SharedObjectTypeCaster::cast(const SharedObject& value,
    pybind11::return_value_policy policy, pybind11::handle parent) {
  return value->inc_ref();
}

bool SharedObjectTypeCaster::load(pybind11::handle source, bool) {
  m_value.emplace(reinterpret_borrow<object>(std::move(source)));
  return true;
}
