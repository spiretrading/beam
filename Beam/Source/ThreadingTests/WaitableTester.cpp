#include <doctest/doctest.h>
#include "Beam/Threading/Waitable.hpp"

using namespace Beam;
using namespace Beam::Routines;
using namespace Beam::Threading;

namespace {
  struct TestWaitable : Waitable {
    bool m_isAvailable;

    TestWaitable()
      : m_isAvailable(false) {}

    void Signal() {
      m_isAvailable = true;
      Notify();
    }

    bool IsAvailable() const override {
      return m_isAvailable;
    }
  };
}

TEST_SUITE("Waitable") {
  TEST_CASE("wait_first") {
    auto a = TestWaitable();
    auto b = TestWaitable();
    auto c = static_cast<TestWaitable*>(nullptr);
    auto signal = std::atomic_bool(false);
    auto r = Spawn([&] {
      c = &Wait(a, b);
      signal = true;
    });
    a.Signal();
    while(!signal) {}
    REQUIRE(c == &a);
  }

  TEST_CASE("wait_second") {
    auto a = TestWaitable();
    auto b = TestWaitable();
    auto c = static_cast<TestWaitable*>(nullptr);
    auto signal = std::atomic_bool(false);
    auto r = Spawn([&] {
      c = &Wait(a, b);
      signal = true;
    });
    b.Signal();
    while(!signal) {}
    REQUIRE(c == &b);
  }
}
