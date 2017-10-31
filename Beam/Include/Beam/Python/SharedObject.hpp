#ifndef BEAM_PYTHON_SHARED_OBJECT_HPP
#define BEAM_PYTHON_SHARED_OBJECT_HPP
#include "Beam/Python/Python.hpp"

namespace Beam {
namespace Python {

  /*! \class SharedObject
      \brief Stores a Python object in a way that it can be safely shared and
             deleted.
   */
  class SharedObject {
    public:

      //! Constructs a SharedObject.
      /*!
        \param object The object to store.
      */
      SharedObject(boost::python::object object);

      //! Returns a reference to the object.
      boost::python::object& operator *() const;

      //! Returns a pointer to the object.
      boost::python::object* operator ->() const;

    private:
      std::shared_ptr<boost::python::object> m_object;
  };

  //! Exports the SharedObject class.
  void ExportSharedObject();
}
}

#endif
