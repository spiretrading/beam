#ifndef BEAM_INSTANTIATE_HPP
#define BEAM_INSTANTIATE_HPP
#include <functional>
#include <stdexcept>
#include <tuple>
#include <typeindex>
#include <utility>
#include <boost/callable_traits/return_type.hpp>
#include <boost/mp11.hpp>
#include <boost/throw_exception.hpp>

namespace Beam {
namespace Details {
  template<typename MetaClass, typename TypeList, std::size_t Index = 0>
  decltype(auto) instantiate_1d(std::type_index type, auto&&... args) {
    using Parameter = boost::mp11::mp_at_c<TypeList, Index>;
    if constexpr(requires {
      MetaClass().template operator()<Parameter>(
        std::forward<decltype(args)>(args)...);
    }) {
      if(typeid(Parameter) == type) {
        return MetaClass().template operator()<Parameter>(
          std::forward<decltype(args)>(args)...);
      }
    }
    if constexpr(Index + 1 >= boost::mp11::mp_size<TypeList>::value) {
      using OperatorType = decltype(&MetaClass::template operator()<Parameter>);
      using ReturnType = boost::callable_traits::return_type_t<OperatorType>;
      return [] () -> ReturnType {
        boost::throw_with_location(
          std::invalid_argument("Type not found in type list."));
      }();
    } else {
      return instantiate_1d<MetaClass, TypeList, Index + 1>(
        type, std::forward<decltype(args)>(args)...);
    }
  }

  template<typename MetaClass, typename TypeList, std::size_t Index = 0,
    typename... TypeInfos>
  decltype(auto) instantiate_2d(
      const std::tuple<TypeInfos...>& types, auto&&... args) {
    const auto PARAMETER_COUNT = sizeof...(TypeInfos);
    using Parameters = boost::mp11::mp_at_c<TypeList, Index>;
    if constexpr(boost::mp11::mp_size<Parameters>::value == PARAMETER_COUNT) {
      if constexpr(requires {
        [&]<std::size_t... Js> (std::index_sequence<Js...>) {
          return MetaClass().template operator()<
            boost::mp11::mp_at_c<Parameters, Js>...>(
              std::forward<decltype(args)>(args)...);
        }(std::make_index_sequence<PARAMETER_COUNT>());
      }) {
        auto is_match = [&]<std::size_t... Js>(std::index_sequence<Js...>) {
          return (... && (typeid(boost::mp11::mp_at_c<Parameters, Js>) ==
            std::get<Js>(types)));
        }(std::make_index_sequence<PARAMETER_COUNT>());
        if(is_match) {
          return [&]<std::size_t... Js> (std::index_sequence<Js...>) ->
              decltype(auto) {
            return MetaClass().template operator()<
              boost::mp11::mp_at_c<Parameters, Js>...>(
                std::forward<decltype(args)>(args)...);
          }(std::make_index_sequence<PARAMETER_COUNT>());
        }
      }
    }
    if constexpr(Index + 1 >= boost::mp11::mp_size<TypeList>::value) {
      using OperatorType = decltype([&]<std::size_t... Js> (
          std::index_sequence<Js...>) {
        return &MetaClass::template operator()<
          boost::mp11::mp_at_c<Parameters, Js>...>;
      }(std::make_index_sequence<boost::mp11::mp_size<Parameters>::value>()));
      using ReturnType = boost::callable_traits::return_type_t<OperatorType>;
      return [] () -> ReturnType {
        boost::throw_with_location(
          std::invalid_argument("Type combination not found in type list."));
      }();
    } else {
      return instantiate_2d<MetaClass, TypeList, Index + 1>(
        types, std::forward<decltype(args)>(args)...);
    }
  }
}

  /**
   * Returns a function template's instantiation.
   * @tparam MetaClass The meta class that defines the template.
   * @param type The template parameter type.
   * @return The function template instantiated with the specified types.
   */
  template<typename MetaClass>
  auto instantiate(std::type_index type) {
    return [=] (auto&&... args) -> decltype(auto) {
      return Details::instantiate_1d<MetaClass, typename MetaClass::type>(
        type, std::forward<decltype(args)>(args)...);
    };
  }

  template<typename MetaClass, typename... TypeInfos>
    requires(sizeof...(TypeInfos) > 1)
  auto instantiate(const TypeInfos&... types) {
    return [types = std::tuple(std::type_index(types)...)] (auto&&... args) ->
        decltype(auto) {
      return Details::instantiate_2d<MetaClass, typename MetaClass::type>(
        types, std::forward<decltype(args)>(args)...);
    };
  }
}

#endif
