#ifndef BEAM_PYTHON_SHARED_OBJECT_HPP
#define BEAM_PYTHON_SHARED_OBJECT_HPP
#include <memory>
#include <pybind11/pybind11.h>
#include "Beam/Python/BasicTypeCaster.hpp"

namespace Beam::Python {

  /**
   * Stores a Python object in a way that it can be safely shared and deleted.
   */
  class SharedObject {
    public:

      /**
       * Constructs a SharedObject.
       * @param object The object to store.
       */
      SharedObject(pybind11::object object);

      /** Returns a reference to the object. */
      pybind11::object& operator *() const;

      /** Returns a pointer to the object. */
      pybind11::object* operator ->() const;

    private:
      std::shared_ptr<pybind11::object> m_object;
  };

  /**
   * Implements a type caster for a SharedObject.
   */
  struct SharedObjectTypeCaster : BasicTypeCaster<SharedObject> {
    static constexpr auto name = pybind11::detail::_("SharedObject");
    static pybind11::handle cast(const SharedObject& value,
      pybind11::return_value_policy policy, pybind11::handle parent);
    bool load(pybind11::handle source, bool);
  };
}

namespace pybind11::detail {
  template<>
  struct type_caster<Beam::Python::SharedObject> :
    Beam::Python::SharedObjectTypeCaster {};
}

#endif
