#include "Beam/Python/SharedObject.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace pybind11;

namespace {
  struct ObjectDeleter {
    void operator ()(object* object) const {
      auto lock = gil_scoped_acquire();
      delete object;
    }
  };
}

SharedObject::SharedObject(object object)
  : m_object(new pybind11::object(std::move(object)), ObjectDeleter()) {}

object& SharedObject::operator *() const {
  return *m_object;
}

object* SharedObject::operator ->() const {
  return m_object.get();
}

handle SharedObjectTypeCaster::cast(const SharedObject& value,
    return_value_policy policy, handle parent) {
  return value->inc_ref();
}

bool SharedObjectTypeCaster::load(handle source, bool) {
  m_value.emplace(reinterpret_borrow<object>(std::move(source)));
  return true;
}
