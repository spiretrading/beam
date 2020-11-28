#ifndef BEAM_CONCEPT_HPP
#define BEAM_CONCEPT_HPP
#include <type_traits>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /** Contains requirements and properties about a template substitution. */
  template<typename T>
  struct Concept {};

  /**
   * Specifies whether a type implements a Concept.
   * @param <I> The type declaring its implementation.
   * @param <C> The type of Concept implemented.
   */
  template<typename I, typename C>
  struct ImplementsConcept : std::false_type {};
}

#endif
