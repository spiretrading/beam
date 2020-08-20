#include "Beam/Python/Utilities.hpp"

using namespace pybind11;

std::ostream& pybind11::operator <<(std::ostream& out, const object& value) {
  return out << str(value).cast<std::string>();
}
