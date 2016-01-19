#ifndef BEAM_CONCEPT_HPP
#define BEAM_CONCEPT_HPP
#include <type_traits>
#include <boost/noncopyable.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \struct Concept
      \brief Contains requirements and properties about a template substitution.
   */
  template<typename T>
  struct Concept : private boost::noncopyable {};

  /*! \struct ImplementsConcept
      \brief Specifies whether a type implements a Concept.
      \tparam InstanceType The type declaring its implementation.
      \tparam ConceptType The type of Concept implemented.
   */
  template<typename InstanceType, typename ConceptType>
  struct ImplementsConcept : std::false_type {};
}

#endif
