module;
#include "Prelude.hpp"

export module Beam:ToString;

import :Value;

export namespace Beam {

  /** Concept satisfied by types that support operator<< with std::ostream. */
  template<typename T>
  concept IsStreamable = requires(std::ostream& out, const T& value) {
    { out << value } -> std::convertible_to<std::ostream&>;
  };

  /**
   * Converts a value to a string using its operator<<.
   * @param value The value to convert.
   */
  template<IsStreamable T>
  std::string to_string(const T& value) {
    return boost::lexical_cast<std::string>(value);
  }
}

