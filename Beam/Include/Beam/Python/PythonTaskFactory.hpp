#ifndef BEAM_PYTHONTASKFACTORY_HPP
#define BEAM_PYTHONTASKFACTORY_HPP
#include <unordered_map>
#include <boost/throw_exception.hpp>
#include "Beam/Python/Python.hpp"
#include "Beam/Tasks/Task.hpp"
#include "Beam/Tasks/TaskPropertyNotFoundException.hpp"

namespace Beam {
namespace Python {

  /*! \class PythonTaskFactory
      \brief Implements a TaskFactory for use within Python.
   */
  class PythonTaskFactory : public Tasks::VirtualTaskFactory {
    public:

      //! Returns a property value.
      /*!
        \param name The name of the property.
        \return The value associated with the <i>name</i>.
      */
      boost::python::object Get(const std::string& name) const;

      //! Sets a property value.
      /*!
        \param name The name of the property.
        \param value The value to set the property to.
      */
      void Set(const std::string& name, const boost::python::object& value);

      virtual boost::any& FindProperty(const std::string& name) override final;

    protected:

      //! Constructs a PythonTaskFactory.
      PythonTaskFactory() = default;

      //! Copies a PythonTaskFactory.
      PythonTaskFactory(const PythonTaskFactory& factory) = default;

      //! Adds a property to this factory.
      /*!
        \param name The name of the property to add.
        \param value The property's default value.
      */
      void DefineProperty(const std::string& name,
        const boost::python::object& value);

    private:
      std::unordered_map<std::string, boost::any> m_properties;
  };

  inline boost::python::object PythonTaskFactory::Get(
      const std::string& name) const {
    return VirtualTaskFactory::Get<boost::python::object>(name);
  }

  inline void PythonTaskFactory::Set(const std::string& name,
      const boost::python::object& value) {
    VirtualTaskFactory::Set(name, value);
  }

  inline boost::any& PythonTaskFactory::FindProperty(const std::string& name) {
    auto propertyIterator = m_properties.find(name);
    if(propertyIterator == m_properties.end()) {
      BOOST_THROW_EXCEPTION(Tasks::TaskPropertyNotFoundException{name});
    }
    return propertyIterator->second;
  }

  inline void PythonTaskFactory::DefineProperty(const std::string& name,
      const boost::python::object& value) {
    m_properties.insert(std::make_pair(name, value));
  }
}
}

#endif
