#include "Beam/Python/SharedObject.hpp"
#include "Beam/Python/GilLock.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace boost;
using namespace boost::python;
using namespace std;

namespace {
  struct ObjectDeleter {
    void operator ()(boost::python::object* object) const {
      GilLock gil;
      boost::lock_guard<GilLock> lock{gil};
      delete object;
    }
  };

  struct SharedObjectToPython {
    static PyObject* convert(const SharedObject& object) {
      return boost::python::incref(object->ptr());
    }
  };
}

void Beam::Python::ExportSharedObject() {
  boost::python::to_python_converter<SharedObject, SharedObjectToPython>();
}

SharedObject::SharedObject(boost::python::object object)
  : m_object{new boost::python::object{std::move(object)}, ObjectDeleter{}} {}

boost::python::object& SharedObject::operator *() const {
  return *m_object;
}

boost::python::object* SharedObject::operator ->() const {
  return m_object.get();
}
