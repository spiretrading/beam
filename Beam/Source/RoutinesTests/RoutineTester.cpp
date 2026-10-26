#include <vector>
#include <doctest/doctest.h>
#include "Beam/Routines/Routine.hpp"

using namespace Beam;

namespace {
  struct RecordingLock {
    Routine*& m_routine;
    std::vector<int>& m_acquisitions;
    int& m_remaining;
    int m_id;

    void lock() {
      m_acquisitions.push_back(m_id);
    }

    void unlock() {
      --m_remaining;
      if(m_remaining == 0) {
        resume(m_routine);
      }
    }
  };
}

TEST_SUITE("Routine") {
  TEST_CASE("suspend_lock_order") {
    auto routine = static_cast<Routine*>(nullptr);
    auto acquisitions = std::vector<int>();
    auto remaining = 3;
    auto first = RecordingLock(routine, acquisitions, remaining, 1);
    auto second = RecordingLock(routine, acquisitions, remaining, 2);
    auto third = RecordingLock(routine, acquisitions, remaining, 3);
    suspend(out(routine), first, second, third);
    REQUIRE(remaining == 0);
    REQUIRE(acquisitions == std::vector({1, 2, 3}));
  }
}
