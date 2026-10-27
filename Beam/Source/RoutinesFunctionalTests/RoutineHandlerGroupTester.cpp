#include <doctest/doctest.h>
#include "Beam/Routines/RoutineHandlerGroup.hpp"

using namespace Beam;

namespace {
  template<typename F>
  concept IsSpawnable = requires(RoutineHandlerGroup group) {
    group.spawn(std::declval<F>());
  };
}

TEST_SUITE("RoutineHandlerGroup") {
  TEST_CASE("spawn_constraints") {
    auto function = [] {};
    REQUIRE(IsSpawnable<decltype(function)>);
    REQUIRE(!IsSpawnable<int>);
  }
}
