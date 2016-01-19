#ifndef BEAM_MATHDETAILS_HPP
#define BEAM_MATHDETAILS_HPP
#include <cmath>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {
namespace Details {
  inline double RoundDouble(double value) {
    if(value >= 0) {
      return std::floor(value + 0.5);
    } else {
      return std::ceil(value + 0.5);
    }
  }
}
}

#endif
