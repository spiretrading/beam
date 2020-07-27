#ifndef BEAM_SHUTTLERECORD_HPP
#define BEAM_SHUTTLERECORD_HPP
#include <boost/call_traits.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/tuple/to_seq.hpp>
#include "Beam/Serialization/Serialization.hpp"
#include "Beam/Utilities/Preprocessor.hpp"

namespace Beam {
namespace Serialization {
  #define BEAM_RECORD_DECLARE_TYPE_LIST_(z, n, i, q)                           \
    BOOST_PP_COMMA_IF(i) BOOST_PP_TUPLE_ELEM(2, 0, q)

  #define BEAM_RECORD_DECLARE_TYPE_LIST(N, ...)                                \
    using TypeList = boost::mpl::vector<BOOST_PP_SEQ_FOR_EACH_I(               \
      BEAM_RECORD_DECLARE_TYPE_LIST_, BOOST_PP_EMPTY,                          \
      BOOST_PP_TUPLE_TO_SEQ(N, (__VA_ARGS__)))>;

  #define BEAM_RECORD_DECLARE_MEMBERS_(z, n, q)                                \
    BOOST_PP_TUPLE_ELEM(2, 0, q) BOOST_PP_TUPLE_ELEM(2, 1, q);

  #define BEAM_RECORD_DECLARE_MEMBERS(N, ...)                                  \
    BOOST_PP_SEQ_FOR_EACH(BEAM_RECORD_DECLARE_MEMBERS_, BOOST_PP_EMPTY,        \
      BOOST_PP_TUPLE_TO_SEQ(N, (__VA_ARGS__)))

  #define BEAM_RECORD_DECLARE_CONSTRUCTOR_(z, n, i, q)                         \
    BOOST_PP_COMMA_IF(i) boost::call_traits<                                   \
      BOOST_PP_TUPLE_ELEM(2, 0, q)>::param_type                                \
      BOOST_PP_CAT(p_, BOOST_PP_TUPLE_ELEM(2, 1, q))

  #define BEAM_RECORD_DECLARE_CONSTRUCTOR(Name, N, ...)                        \
    Name(BOOST_PP_SEQ_FOR_EACH_I(BEAM_RECORD_DECLARE_CONSTRUCTOR_,             \
      BOOST_PP_EMPTY, BOOST_PP_TUPLE_TO_SEQ(N, (__VA_ARGS__))))

  #define BEAM_RECORD_INITIALIZE_MEMBERS_(z, n, i, q)                          \
    BOOST_PP_COMMA_IF(i) BOOST_PP_TUPLE_ELEM(2, 1, q)                          \
      (BOOST_PP_CAT(p_, BOOST_PP_TUPLE_ELEM(2, 1, q)))

  #define BEAM_RECORD_INITIALIZE_MEMBERS(N, ...)                               \
    : BOOST_PP_SEQ_FOR_EACH_I(BEAM_RECORD_INITIALIZE_MEMBERS_,                 \
      BOOST_PP_EMPTY, BOOST_PP_TUPLE_TO_SEQ(N, (__VA_ARGS__))) {}

  #define BEAM_RECORD_SHUTTLE_MEMBERS(z, n, q)                                 \
    shuttle.Shuttle(BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2, 1, q)),          \
      BOOST_PP_TUPLE_ELEM(2, 1, q));

  #define BEAM_RECORD_DEFINE_SHUTTLE(N, ...)                                   \
    template<typename Shuttler>                                                \
    void Shuttle(Shuttler& shuttle, unsigned int version) {                    \
      BOOST_PP_SEQ_FOR_EACH(BEAM_RECORD_SHUTTLE_MEMBERS, Name,                 \
        BOOST_PP_TUPLE_TO_SEQ(N, (__VA_ARGS__)))                               \
    }

  #define BEAM_RECORD_DEFINE_GETTERS_(z, Name, i, q)                           \
    template<>                                                                 \
    inline boost::call_traits<boost::mpl::at_c<                                \
        Name::TypeList, i>::type>::reference Name::Get<i>() {                  \
      return BOOST_PP_TUPLE_ELEM(2, 1, q);                                     \
    }                                                                          \
                                                                               \
    template<>                                                                 \
    inline boost::call_traits<boost::mpl::at_c<                                \
        Name::TypeList, i>::type>::const_reference Name::Get<i>() const {      \
      return BOOST_PP_TUPLE_ELEM(2, 1, q);                                     \
    }

  #define BEAM_RECORD_DEFINE_GETTERS(N, Name, ...)                             \
    BOOST_PP_SEQ_FOR_EACH_I(BEAM_RECORD_DEFINE_GETTERS_, Name,                 \
      BOOST_PP_TUPLE_TO_SEQ(N, (__VA_ARGS__)))

  #define BEAM_DEFINE_RECORD__(Name, N, ...)                                   \
    struct Name {                                                              \
      BEAM_RECORD_DECLARE_TYPE_LIST(N, __VA_ARGS__)                            \
                                                                               \
      BEAM_RECORD_DECLARE_MEMBERS(N, __VA_ARGS__)                              \
                                                                               \
      Name() {}                                                                \
                                                                               \
      BEAM_RECORD_DECLARE_CONSTRUCTOR(Name, N, __VA_ARGS__)                    \
       BEAM_RECORD_INITIALIZE_MEMBERS(N, __VA_ARGS__)                          \
                                                                               \
      BEAM_RECORD_DEFINE_SHUTTLE(N, __VA_ARGS__)                               \
                                                                               \
      template<int I>                                                          \
      inline typename boost::call_traits<                                      \
          typename boost::mpl::at_c<TypeList, I>::type>::reference Get() {     \
        return typename boost::mpl::at_c<TypeList, I>::type();                 \
      };                                                                       \
                                                                               \
      template<int I>                                                          \
      inline typename boost::call_traits<                                      \
          typename boost::mpl::at_c<TypeList, I>::type>::const_reference       \
          Get() const {                                                        \
        return typename boost::mpl::at_c<TypeList, I>::type();                 \
      };                                                                       \
    };                                                                         \
                                                                               \
    BEAM_RECORD_DEFINE_GETTERS(N, Name, __VA_ARGS__)

  #define BEAM_DEFINE_RECORD_(Name, ...)                                       \
    BEAM_DEFINE_RECORD__(Name, PP_NARG(__VA_ARGS__), __VA_ARGS__)

  #define BEAM_DEFINE_EMPTY_RECORD(Name)                                       \
    struct Name {                                                              \
      using TypeList = boost::mpl::vector<>;                                   \
                                                                               \
      Name() {}                                                                \
                                                                               \
      template<typename Shuttler>                                              \
      void Shuttle(Shuttler& shuttle, unsigned int version) {}                 \
                                                                               \
      template<int I>                                                          \
      typename boost::call_traits<                                             \
          typename boost::mpl::at_c<TypeList, I>::type>::reference Get() {     \
        return typename boost::mpl::at_c<TypeList, I>::type();                 \
      };                                                                       \
                                                                               \
      template<int I>                                                          \
      typename boost::call_traits<                                             \
          typename boost::mpl::at_c<TypeList, I>::type>::const_reference       \
          Get() const {                                                        \
        return typename boost::mpl::at_c<TypeList, I>::type();                 \
      };                                                                       \
    };

  #define BEAM_EXPAND_RECORD(...) __VA_ARGS__
  #define BEAM_DEFINE_RECORD(Name, ...)                                        \
    BOOST_PP_EXPAND(BEAM_EXPAND_RECORD                                         \
      BOOST_PP_IF(BEAM_PP_NARG_IS_EMPTY(__VA_ARGS__),                          \
        (BEAM_DEFINE_EMPTY_RECORD(Name)),                                      \
        (BEAM_DEFINE_RECORD_(Name, MAKE_PAIRS(__VA_ARGS__)))))
}
}

#endif
