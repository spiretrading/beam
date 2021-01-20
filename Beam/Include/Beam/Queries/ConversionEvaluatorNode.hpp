#ifndef BEAM_CONVERSION_EVALUATOR_NODE_HPP
#define BEAM_CONVERSION_EVALUATOR_NODE_HPP
#include <utility>
#include "Beam/Queries/FunctionEvaluatorNode.hpp"
#include "Beam/Queries/Queries.hpp"

namespace Beam::Queries {
namespace Details {
  template<typename Source, typename Target>
  struct CastFunctor {
    Target operator ()(const Source& source) const {
      return static_cast<Target>(source);
    }
  };

  template<typename Source, typename Target>
  struct ConstructFunctor {
    Target operator ()(const Source& source) const {
      return {source};
    }
  };
}

  /**
   * Makes an EvaluatorNode that casts from one value to another.
   * @param arg The EvaluatorNode providing the value to cast.
   */
  template<typename Source, typename Target, typename Arg>
  std::unique_ptr<FunctionEvaluatorNode<Details::CastFunctor<Source, Target>>>
      MakeCastEvaluatorNode(std::unique_ptr<Arg> arg) {
    return MakeFunctionEvaluatorNode(
      Details::CastFunctor<Source, Target>{}, std::move(arg));
  }

  /**
   * Makes an EvaluatorNode that constructs a value.
   * @param arg The EvaluatorNode providing the value to convert.
   */
  template<typename Source, typename Target, typename Arg>
  std::unique_ptr<
    FunctionEvaluatorNode<Details::ConstructFunctor<Source, Target>>>
      MakeConstructEvaluatorNode(std::unique_ptr<Arg> arg) {
    return MakeFunctionEvaluatorNode(
      Details::ConstructFunctor<Source, Target>{}, std::move(arg));
  }
}

#endif
