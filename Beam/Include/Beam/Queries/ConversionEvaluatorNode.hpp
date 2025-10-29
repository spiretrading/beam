#ifndef BEAM_CONVERSION_EVALUATOR_NODE_HPP
#define BEAM_CONVERSION_EVALUATOR_NODE_HPP
#include <utility>
#include "Beam/Queries/FunctionEvaluatorNode.hpp"

namespace Beam {

  /**
   * Makes an EvaluatorNode that casts from one value to another.
   * @param arg The EvaluatorNode providing the value to cast.
   */
  template<typename Source, typename Target, typename Arg>
  auto make_cast_evaluator_node(std::unique_ptr<Arg> arg) {
    return make_function_evaluator_node([] (const Source& source) {
      return static_cast<Target>(source);
    }, std::move(arg));
  }

  /**
   * Makes an EvaluatorNode that constructs a value.
   * @param arg The EvaluatorNode providing the value to convert.
   */
  template<typename Source, typename Target, typename Arg>
  auto make_construct_evaluator_node(std::unique_ptr<Arg> arg) {
    return make_function_evaluator_node([] (const Source& source) {
      return Target(source);
    }, std::move(arg));
  }
}

#endif
