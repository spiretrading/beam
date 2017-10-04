#ifndef BEAM_PYTHON_SIGNALS_SLOTS_HPP
#define BEAM_PYTHON_SIGNALS_SLOTS_HPP
#include <boost/python.hpp>
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {
namespace Details {
  template<typename... Args>
  struct SlotFromPythonConverter {
    static void* convertible(PyObject* object) {
      if(PyCallable_Check(object)) {
        return object;
      }
      return nullptr;
    }

    static void construct(PyObject* object,
        boost::python::converter::rvalue_from_python_stage1_data* data) {
      using Slot = boost::signals2::signal<void (Args...)>::slot_type;
      auto storage = reinterpret_cast<boost::python::converter::
        rvalue_from_python_storage<Slot>*>(data)->storage.bytes;
      boost::python::handle<> handle{boost::python::borrowed(object)};
      boost::python::object slot{handle};
      new(storage) Slot{
        [=] (Args&&... args) {
          GilLock gil;
          boost::lock_guard<GilLock> lock{gil};
          slot(std::forward<Args>(args)...);
        }
      };
      data->convertible = storage;
    }
  };
}

  //! Exports the boost::signals2::connection class.
  void ExportBoostConnection();

  //! Exports the boost::signals2::scoped_connection class.
  void ExportBoostScopedConnection();

  //! Exports all boost::signals2 classes.
  void ExportSignalsSlots();

  //! Exports a boost signal.
  /*!
    \param name The name to give to the class from within Python.
  */
  template<typename... Args>
  void ExportSignal(const char* name) {
    using Slot = boost::signals2::signal<void (Args...)>::slot_type;
    auto typeId = boost::python::type_id<Slot>();
    auto registration = boost::python::converter::registry::query(typeId);
    if(registration != nullptr && registration->m_to_python != nullptr) {
      return;
    }
    boost::python::converter::registry::push_back(
      &Details::SlotFromPythonConverter<Args...>::convertible,
      &Details::SlotFromPythonConverter<Args...>::construct,
      boost::python::type_id<Slot>());
  }
}
}

#endif
