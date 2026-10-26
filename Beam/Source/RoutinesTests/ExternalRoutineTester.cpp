#include <doctest/doctest.h>
#include "Beam/Routines/ExternalRoutine.hpp"

using namespace Beam;

TEST_SUITE("ExternalRoutine") {
  TEST_CASE("early_resume") {
    auto external = ExternalRoutine();
    auto& routine = static_cast<Routine&>(external);
    routine.execute();
    routine.pending_suspend();
    routine.resume();
    routine.suspend();
    REQUIRE(routine.get_state() == Routine::State::RUNNING);
  }
}
