#include "Beam/Python/ToPythonTask.hpp"
#include <string>
#include <unordered_map>
#include "Beam/Tasks/TaskPropertyNotFoundException.hpp"

using namespace Beam;
using namespace Beam::Python;
using namespace Beam::Tasks;
using namespace std;

namespace {
  std::unordered_map<string, BasePythonTaskFactoryProperty*> properties;
}

void Beam::Tasks::Details::AddTaskFactoryProperty(const char* name,
    void* property) {
  std::string properName{name};
  auto typedProperty = static_cast<BasePythonTaskFactoryProperty*>(property);
  if(properties.find(properName) != properties.end()) {
    delete typedProperty;
    return;
  }
  properties.insert(std::make_pair(properName, typedProperty));
}

boost::python::object Beam::Tasks::GetTaskFactoryProperty(
    VirtualTaskFactory* factory, const char* name) {
  auto& value = factory->FindProperty(name);
  auto propertyIterator = properties.find(value.type().name());
  if(propertyIterator == properties.end()) {
    return boost::any_cast<boost::python::object>(value);
  }
  return propertyIterator->second->Get(value);
}

void Beam::Tasks::SetTaskFactoryProperty(VirtualTaskFactory* factory,
    const char* name, const boost::python::object* value) {
  std::string properName{name};
  auto& property = factory->FindProperty(properName);
  auto propertyIterator = properties.find(property.type().name());
  if(propertyIterator == properties.end()) {
    factory->Set(properName, *value);
  } else {
    propertyIterator->second->Set(*factory, properName, *value);
  }
}
