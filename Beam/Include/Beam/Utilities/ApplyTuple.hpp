#ifndef BEAM_APPLYTUPLE_HPP
#define BEAM_APPLYTUPLE_HPP
#include <tuple>
#include <utility>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \struct IntegerSequence
      \brief Stores an increasing sequence of ints.
   */
  template<int ...>
  struct IntegerSequence {};

  /*! \struct IntegerSequenceGenerator
      \brief Generates a sequence of ints.
   */
  template<int N, int... S>
  struct IntegerSequenceGenerator :
    IntegerSequenceGenerator<N - 1, N - 1, S...> {};

  template<int... S>
  struct IntegerSequenceGenerator<0, S...> {
    using type = IntegerSequence<S...>;
  };

  template<typename F, typename Tuple, int... Sequence>
  decltype(auto) Invoke(IntegerSequence<Sequence...> sequence, F&& f,
      const Tuple& args) {
    return f(std::get<Sequence>(args)...);
  }

  template<typename F, typename Tuple, int... Sequence>
  decltype(auto) Invoke(IntegerSequence<Sequence...> sequence, F&& f,
      Tuple&& args) {
    return f(std::get<Sequence>(std::move(args))...);
  }

  //! Applies a tuple to a function.
  /*!
    \param args The tuple representing the parameters to pass to the function.
    \param f The function to call.
  */
  template<typename F, typename... Args>
  decltype(auto) Apply(const std::tuple<Args...>& args, F&& f) {
    using Sequence = typename IntegerSequenceGenerator<sizeof...(Args)>::type;
    return Invoke(Sequence(), std::forward<F>(f), args);
  }

  //! Applies a tuple to a function.
  /*!
    \param args The tuple representing the parameters to pass to the function.
    \param f The function to call.
  */
  template<typename F, typename... Args>
  decltype(auto) Apply(std::tuple<Args...>&& args, F&& f) {
    using Sequence = typename IntegerSequenceGenerator<sizeof...(Args)>::type;
    return Invoke(Sequence(), std::forward<F>(f), std::move(args));
  }
}

#endif
