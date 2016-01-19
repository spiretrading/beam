#ifndef BEAM_VARIANTLAMBDAVISITOR_HPP
#define BEAM_VARIANTLAMBDAVISITOR_HPP
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/empty.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repeat.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include "Beam/Utilities/Preprocessor.hpp"
#include "Beam/Utilities/Utilities.hpp"

#define BEAM_VARIANT_LAMBDA_VISITOR_PARAMETERS 10

namespace Beam {

  /*! \class VariantLambdaVisitor
      \brief Generates a static_visitor for a boost::variant.
   */
  template<typename ReturnType, BOOST_PP_ENUM_BINARY_PARAMS(
    BEAM_VARIANT_LAMBDA_VISITOR_PARAMETERS, typename Lambda,
    = NullType BOOST_PP_INTERCEPT), typename DummyType = NullType> 
  struct VariantLambdaVisitor {};

  #define BEAM_INHERIT_LAMBDA(z, n, q) BOOST_PP_COMMA_IF(n) public Lambda##n
  #define BEAM_USING_LAMBDA(z, n, q) using Lambda##n::operator();
  #define BEAM_CONSTRUCT_LAMBDA(z, n, q)                                       \
    BOOST_PP_COMMA_IF(n) Lambda##n(std::forward<LambdaForward##n>(l##n))
  #define BEAM_REMOVE_REFERENCE(z, n, q)                                       \
    BOOST_PP_COMMA_IF(n) typename std::remove_reference<Lambda##n>::type
  #define BEAM_PASS_LAMBDA(z, n, q)                                            \
    BOOST_PP_COMMA_IF(n) std::forward<Lambda##n>(l##n)

  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<typename ReturnType BOOST_PP_COMMA_IF(n)                            \
    BOOST_PP_ENUM_PARAMS(n, typename Lambda)>                                  \
  struct VariantLambdaVisitor<ReturnType BOOST_PP_COMMA_IF(n)                  \
      BOOST_PP_ENUM_PARAMS(n, Lambda)> : public boost::static_visitor<         \
      ReturnType> BOOST_PP_COMMA_IF(n)                                         \
      BOOST_PP_REPEAT(n, BEAM_INHERIT_LAMBDA, BOOST_PP_EMPTY) {                \
    BOOST_PP_REPEAT(n, BEAM_USING_LAMBDA, BOOST_PP_EMPTY)                      \
                                                                               \
    template<BOOST_PP_ENUM_PARAMS(n, typename LambdaForward)>                  \
    VariantLambdaVisitor(BOOST_PP_ENUM_BINARY_PARAMS(n, LambdaForward, && l)); \
  };                                                                           \
                                                                               \
  template<typename ReturnType BOOST_PP_COMMA_IF(n)                            \
    BOOST_PP_ENUM_PARAMS(n, typename Lambda)>                                  \
  template<BOOST_PP_ENUM_PARAMS(n, typename LambdaForward)>                    \
  VariantLambdaVisitor<ReturnType BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, \
      Lambda)>::VariantLambdaVisitor(BOOST_PP_ENUM_BINARY_PARAMS(n,            \
      LambdaForward, && l))                                                    \
      : boost::static_visitor<ReturnType>() BOOST_PP_COMMA_IF(n)               \
        BOOST_PP_REPEAT(n, BEAM_CONSTRUCT_LAMBDA, BOOST_PP_EMPTY) {}           \
                                                                               \
  template<typename ReturnType BOOST_PP_COMMA_IF(n)                            \
    BOOST_PP_ENUM_PARAMS(n, typename Lambda)>                                  \
  VariantLambdaVisitor<ReturnType BOOST_PP_COMMA_IF(n)                         \
      BOOST_PP_REPEAT(n, BEAM_REMOVE_REFERENCE, BOOST_PP_EMPTY)>               \
      MakeVariantLambdaVisitor(BOOST_PP_ENUM_BINARY_PARAMS(n, Lambda, && l)) { \
    return VariantLambdaVisitor<ReturnType BOOST_PP_COMMA_IF(n)                \
      BOOST_PP_REPEAT(n, BEAM_REMOVE_REFERENCE, BOOST_PP_EMPTY)>(              \
      BOOST_PP_REPEAT(n, BEAM_PASS_LAMBDA, BOOST_PP_EMPTY));                   \
  }                                                                            \
                                                                               \
  template<typename ReturnType, typename V BOOST_PP_COMMA_IF(n)                \
    BOOST_PP_ENUM_PARAMS(n, typename Lambda)>                                  \
  ReturnType ApplyVariantLambdaVisitor(const V& v BOOST_PP_COMMA_IF(n)         \
      BOOST_PP_ENUM_BINARY_PARAMS(n, Lambda, && l)) {                          \
    return boost::apply_visitor(MakeVariantLambdaVisitor<ReturnType>(          \
      BOOST_PP_REPEAT(n, BEAM_PASS_LAMBDA, BOOST_PP_EMPTY)), v);               \
  }

  #define BOOST_PP_LOCAL_LIMITS (1, BEAM_VARIANT_LAMBDA_VISITOR_PARAMETERS)
  #include BOOST_PP_LOCAL_ITERATE()
  #undef BEAM_PASS_REFERENCE
  #undef BEAM_REMOVE_REFERENCE
  #undef BEAM_CONSTRUCT_LAMBDA
  #undef BEAM_USING_LAMBDA
  #undef BEAM_INHERIT_LAMBDA
}

#endif
