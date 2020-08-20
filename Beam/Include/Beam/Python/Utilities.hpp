#include <ostream>
#include <pybind11/pybind11.h>

namespace pybind11 {
  std::ostream& operator <<(std::ostream& out, const object& value);
}
