#ifndef BEAM_SHUTTLE_RECORD_HPP
#define BEAM_SHUTTLE_RECORD_HPP
#include <tuple>
#include <type_traits>
#include <utility>
#include "Beam/Utilities/Preprocessor.hpp"

namespace Beam::Details {
  template<typename T, typename C>
  struct RecordFieldEntry {
    const char* m_name;
    T C::* m_ptr;
  };

  template<class C, class Tuple, std::size_t... I, class F>
  constexpr void for_each_field(
      C& c, const Tuple& t, std::index_sequence<I...>, F&& f) {
    ((f(std::get<I>(t).m_name, c.*(std::get<I>(t).m_ptr))), ...);
  }

  template<class C, class Tuple, class F>
  constexpr void for_each_field(C& c, const Tuple& t, F&& f) {
    constexpr auto N = std::tuple_size_v<std::remove_cvref_t<Tuple>>;
    Beam::Details::for_each_field(
      c, t, std::make_index_sequence<N>(), std::forward<F>(f));
  }

  template<class C, class Tuple, std::size_t I>
  constexpr decltype(auto) get_field(C& c, const Tuple& t) {
    return c.*(std::get<I>(t).m_ptr);
  }

  #define BEAM_DECLARE_MEMBER(Name, T, n) T n;
  #define BEAM_STRINGIFY(x) #x
  #define BEAM_MAKE_ENTRY(Name, T, n)                                          \
    ::Beam::Details::RecordFieldEntry<T, Name>(BEAM_STRINGIFY(n), &Name::n),
}

#define BEAM_DEFINE_RECORD(Name, ...)                                          \
  struct Name {                                                                \
    __VA_OPT__(BEAM_PP_FOR_EACH_PAIRS(BEAM_DECLARE_MEMBER, Name, __VA_ARGS__)) \
                                                                               \
    template<IsShuttle S>                                                      \
    void shuttle(S& shuttle, unsigned int version) {                           \
      static constexpr auto fields = std::tuple{                               \
        __VA_OPT__(BEAM_PP_FOR_EACH_PAIRS(BEAM_MAKE_ENTRY, Name, __VA_ARGS__)) \
      };                                                                       \
      Beam::Details::for_each_field(                                           \
        *this, fields, [&] (auto key, auto& value) {                           \
          shuttle.shuttle(key, value);                                         \
        });                                                                    \
    }                                                                          \
                                                                               \
    template<std::size_t I>                                                    \
    decltype(auto) get() const {                                               \
      return std::as_const(const_cast<Name*>(this)->template get<I>());        \
    }                                                                          \
                                                                               \
    template<std::size_t I>                                                    \
    decltype(auto) get() {                                                     \
      static constexpr auto fields = std::tuple{                               \
        __VA_OPT__(BEAM_PP_FOR_EACH_PAIRS(BEAM_MAKE_ENTRY, Name, __VA_ARGS__)) \
      };                                                                       \
      using Fields = std::remove_cvref_t<decltype(fields)>;                    \
      static_assert(I < std::tuple_size_v<Fields>, "get<I> out of range");     \
      return Beam::Details::get_field<Name, Fields, I>(*this, fields);         \
    }                                                                          \
  };

#endif
