#ifndef BEAM_FUNCTIONAL_HPP
#define BEAM_FUNCTIONAL_HPP
#include <functional>
#include <type_traits>
#include <utility>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/typeof/typeof.hpp>

namespace Beam {
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
}

#endif
