#ifndef BEAM_PREPROCESSOR_HPP
#define BEAM_PREPROCESSOR_HPP
#include <boost/preprocessor/comparison/equal.hpp>
#include <boost/preprocessor/facilities/expand.hpp>

#ifdef _MSC_VER
  #define PP_NARG(...) BOOST_PP_EXPAND(PP_NARG_(__VA_ARGS__, PP_RSEQ_N()))
#else
  #define PP_NARG(...) PP_NARG_(__VA_ARGS__, PP_RSEQ_N())
#endif

#ifdef _MSC_VER
  #define PP_NARG_(...) BOOST_PP_EXPAND(PP_ARG_N(__VA_ARGS__))
#else
  #define PP_NARG_(...) PP_ARG_N(__VA_ARGS__)
#endif

#define PP_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14,  \
  _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29,   \
  _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44,   \
  _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55, _56, _57, _58, _59,   \
  _60, _61, _62, _63, N, ...) N

#define PP_RSEQ_N()                                                            \
  63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45,  \
  44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26,  \
  25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6,  \
  5, 4, 3, 2, 1, 0

#define MAKE_PAIRS_2(_1, _2) (_1, _2)
#define MAKE_PAIRS_4(_1, _2, _3, _4) MAKE_PAIRS_2(_1, _2), MAKE_PAIRS_2(_3, _4)
#define MAKE_PAIRS_6(_1, _2, _3, _4, _5, _6) MAKE_PAIRS_4(_1, _2, _3, _4),     \
  MAKE_PAIRS_2(_5, _6)
#define MAKE_PAIRS_8(_1, _2, _3, _4, _5, _6, _7, _8)                           \
  MAKE_PAIRS_6(_1, _2, _3, _4, _5, _6), MAKE_PAIRS_2(_7, _8)
#define MAKE_PAIRS_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10)                 \
  MAKE_PAIRS_8(_1, _2, _3, _4, _5, _6, _7, _8), MAKE_PAIRS_2(_9, _10)
#define MAKE_PAIRS_12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12)       \
  MAKE_PAIRS_10(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10), MAKE_PAIRS_2(_11, _12)
#define MAKE_PAIRS_14(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13,  \
  _14) MAKE_PAIRS_12(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12),       \
  MAKE_PAIRS_2(_13, _14)

#ifdef _MSC_VER
  #define MAKE_PAIRS__(N, ...) BOOST_PP_EXPAND(MAKE_PAIRS_ ## N (__VA_ARGS__))
#else
  #define MAKE_PAIRS__(N, ...) MAKE_PAIRS_ ## N (__VA_ARGS__)
#endif

#define MAKE_PAIRS_(N, ...) MAKE_PAIRS__(N, __VA_ARGS__)
#define MAKE_PAIRS(...) MAKE_PAIRS_(PP_NARG(__VA_ARGS__), __VA_ARGS__)

#ifdef _MSC_VER
  #define BEAM_PP_NARG_IS_EMPTY(...)                                           \
    BOOST_PP_EQUAL(PP_NARG(BEAM_DUMMY, __VA_ARGS__),                           \
    PP_NARG(BEAM_DUMMY, BEAM_DUMMY_2, __VA_ARGS__))
#else
  #define BEAM_PP_NARG_IS_EMPTY(...)                                           \
    BOOST_PP_EQUAL(PP_NARG(BEAM_DUMMY, ##__VA_ARGS__), PP_NARG(BEAM_DUMMY))
#endif
#endif
