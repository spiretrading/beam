#ifndef BEAM_HASHTUPLE_HPP
#define BEAM_HASHTUPLE_HPP
#include <tuple>
#include <boost/functional/hash/hash.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/tuple/tuple.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace std {
  #define BEAM_HASH_TUPLE_SIZE 5

  #define BEAM_HASH_TUPLE_MEMBER(z, n, a)                                      \
    boost::hash_combine(seed, std::get<n>(value));

  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<BOOST_PP_ENUM_PARAMS(n, typename A)>                                \
  struct hash<std::tuple<BOOST_PP_ENUM_PARAMS(n, A)>> {                        \
    std::size_t operator ()(const std::tuple<BOOST_PP_ENUM_PARAMS(n, A)>&      \
        value) const {                                                         \
      std::size_t seed = 0;                                                    \
      BOOST_PP_REPEAT(n, BEAM_HASH_TUPLE_MEMBER, BOOST_PP_EMPTY);              \
      return seed;                                                             \
    }                                                                          \
  };

  #define BOOST_PP_LOCAL_LIMITS (0, BEAM_HASH_TUPLE_SIZE)
  #include BOOST_PP_LOCAL_ITERATE()
  #undef BEAM_HASH_TUPLE_MEMBER

  #define BEAM_HASH_TUPLE_MEMBER(z, n, a)                                      \
    boost::hash_combine(seed, boost::get<n>(value));

  #define BOOST_PP_LOCAL_MACRO(n)                                              \
  template<BOOST_PP_ENUM_PARAMS(n, typename A)>                                \
  struct hash<boost::tuple<BOOST_PP_ENUM_PARAMS(n, A)>> {                      \
    std::size_t operator ()(const boost::tuple<BOOST_PP_ENUM_PARAMS(n, A)>&    \
        value) const {                                                         \
      std::size_t seed = 0;                                                    \
      BOOST_PP_REPEAT(n, BEAM_HASH_TUPLE_MEMBER, BOOST_PP_EMPTY);              \
      return seed;                                                             \
    }                                                                          \
  };

  #define BOOST_PP_LOCAL_LIMITS (0, BEAM_HASH_TUPLE_SIZE)
  #include BOOST_PP_LOCAL_ITERATE()
  #undef BEAM_HASH_TUPLE_MEMBER
}

#endif
