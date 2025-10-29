#ifndef BEAM_VARIANT_LAMBDA_VISITOR_HPP
#define BEAM_VARIANT_LAMBDA_VISITOR_HPP
#include <type_traits>
#include <utility>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/variant.hpp>

namespace Beam {

  /**
   * Combines multiple callables into a single overload set usable with
   * boost::apply_visitor.
   * @tparam Ts The types of callables to combine.
   */
  template<typename... Ts>
  struct VariantLambdaVisitor;

  template<typename... Ts>
  struct VariantLambdaVisitor : public Ts... {
    VariantLambdaVisitor(Ts&&... lambdas);

    using Ts::operator()...;
  };

  template<typename... Ts>
  VariantLambdaVisitor<Ts...>::VariantLambdaVisitor(Ts&&... lambdas)
    : Ts(std::forward<Ts>(lambdas))... {}

  /**
   * Constructs a VariantLambdaVisitor by decaying callable types.
   * @param lambdas The callables to combine.
   */
  template<typename... Ts>
  auto make_variant_lambda_visitor(Ts&&... lambdas) {
    return VariantLambdaVisitor<std::decay_t<Ts>...>(
      std::forward<Ts>(lambdas)...);
  }

  /**
   * Applies the provided callables to a boost::variant via
   * boost::apply_visitor.
   * @param variant The variant to visit.
   * @param lambdas The callables to apply to the variant.
   */
  template<typename V, typename... Ts>
  auto apply_variant_lambda_visitor(const V& variant, Ts&&... lambdas) {
    return boost::apply_visitor(
      make_variant_lambda_visitor(std::forward<Ts>(lambdas)...), variant);
  }
}

#endif
