#ifndef BEAM_QUERIES_VALUE_SHUTTLE_TESTS_HPP
#define BEAM_QUERIES_VALUE_SHUTTLE_TESTS_HPP
#include "Beam/Queries/ShuttleQueryTypes.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

namespace Beam::Tests {
  template<typename T, std::invocable<T&&> F>
  void test_query_round_trip_shuttle(const T& value, F&& f) {
    test_polymorphic_round_trip_shuttle(value, [] (auto registry) {
      register_query_types(out(registry));
    }, std::forward<F>(f));
  }

  template<std::equality_comparable T>
  void test_query_round_trip_shuttle(const T& value) {
    test_query_round_trip_shuttle(value, [&] (auto&& received) {
      REQUIRE(value == received);
    });
  }
}

#endif
