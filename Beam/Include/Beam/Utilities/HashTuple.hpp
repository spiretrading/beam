#ifndef BEAM_HASH_TUPLE_HPP
#define BEAM_HASH_TUPLE_HPP
#include <tuple>
#include <boost/functional/hash/hash.hpp>

namespace Beam {
namespace Details {
  template<typename T, std::size_t N>
  struct hash_combine;

  template<typename... T, std::size_t N>
  struct hash_combine<std::tuple<T...>, N> {
    void operator ()(std::size_t& seed, const std::tuple<T...>& value) const {
      hash_combine<std::tuple<T...>, N - 1>()(seed, value);
      boost::hash_combine(seed,
        std::hash<typename std::tuple_element<N, std::tuple<T...>>::type>()(
          std::get<N>(value)));
    }
  };

  template<typename... T>
  struct hash_combine<std::tuple<T...>, 0> {
    void operator ()(std::size_t& seed, const std::tuple<T...>& value) const {
      boost::hash_combine(seed,
        std::hash<typename std::tuple_element<0, std::tuple<T...>>::type>()(
          std::get<0>(value)));
    }
  };
};
}

namespace std {
  template<typename... T>
  struct hash<std::tuple<T...>> {
    std::size_t operator ()(const std::tuple<T...>& value) const {
      auto seed = std::size_t(0);
      Beam::Details::hash_combine<std::tuple<T...>, sizeof...(T) - 1>()(
        seed, value);
      return seed;
    }
  };
}

#endif
