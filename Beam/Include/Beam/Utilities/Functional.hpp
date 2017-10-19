#ifndef BEAM_FUNCTIONAL_HPP
#define BEAM_FUNCTIONAL_HPP
#include <functional>
#include <type_traits>
#include <utility>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/empty.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/type_traits/is_function.hpp>
#include <boost/type_traits/is_member_function_pointer.hpp>
#include <boost/typeof/typeof.hpp>

namespace Beam {

  template<typename... Types>
  struct TypeSequence {};

namespace Details {
  template<typename F, bool IsClass>
  struct GetSignatureHelper {
    using type = typename boost::function_types::function_type<F>::type;
  };

  template<typename F>
  struct GetClassSignatureHelper {
    using R = typename GetSignatureHelper<F, false>::type;
    using result_type = typename boost::function_types::result_type<R>::type;
    using parameter_types =
      typename boost::function_types::parameter_types<R>::type;
    using base = typename boost::mpl::pop_front<parameter_types>::type;
    using L = typename boost::mpl::push_front<base, result_type>::type;
    using type = typename boost::function_types::function_type<L>::type;
  };

  template<typename F>
  struct GetSignatureHelper<F, true> {
    using FT = BOOST_TYPEOF_TPL(&F::operator());
    using type = typename GetClassSignatureHelper<FT>::type;
  };

  template<typename T1, typename T2>
  struct Append {};

  template<template<typename... A1> class T1, template<typename... A2> class T2,
    typename... P1, typename... P2>
  struct Append<T1<P1...>, T2<P2...>> {
    using type = TypeSequence<P1..., P2...>;
  };

  template<typename V, bool IsEmpty = boost::mpl::empty<V>::value>
  struct MakeTypeSequence {};

  template<typename V>
  struct MakeTypeSequence<V, false> {
    using type = typename Append<
      TypeSequence<typename boost::mpl::front<V>::type>,
      typename MakeTypeSequence<
      typename boost::mpl::pop_front<V>::type>::type>::type;
  };

  template<typename V>
  struct MakeTypeSequence<V, true> {
    using type = TypeSequence<>;
  };

  template<typename F, bool IsFunction =
    boost::is_function<typename std::decay<F>::type>::value ||
    boost::is_member_function_pointer<F>::value>
  struct FunctionParametersHelper {};

  template<typename F>
  struct FunctionParametersHelper<F, true> {
    using type = typename Details::MakeTypeSequence<
      typename boost::function_types::parameter_types<
      typename std::decay<F>::type>::type>::type;
  };

  template<typename F>
  struct FunctionParametersHelper<F, false> {
    using type = typename FunctionParametersHelper<
      decltype(&F::operator ())>::type;
  };
}

  template<typename F>
  struct GetSignature {
    using type = typename Details::GetSignatureHelper<
      F, std::is_class<F>::value>::type;
  };

  template<typename F, typename... Args>
  struct ResultOf {
    using type = decltype(std::declval<F>()(std::declval<Args>()...));
  };

  template<typename F, typename... Args>
  using GetResultOf = typename ResultOf<F, Args...>::type;

  template<typename F>
  struct FunctionParameters {
    using type = typename Details::FunctionParametersHelper<F>::type;
  };

  template<typename F>
  using GetFunctionParameters = typename FunctionParameters<F>::type;
}

#endif
