#include <doctest/doctest.h>
#include "Beam/Threading/LockRelease.hpp"

using namespace Beam;

namespace {
  struct dummy_lock {
    bool m_is_locked = true;

    void lock() {
      m_is_locked = true;
    }

    void unlock() {
      m_is_locked = false;
    }
  };
}

TEST_SUITE("LockRelease") {
  TEST_CASE("construction_releases_lock") {
    auto lock = dummy_lock();
    REQUIRE(lock.m_is_locked);
    {
      auto release = LockRelease(lock);
      REQUIRE(!lock.m_is_locked);
    }
    REQUIRE(lock.m_is_locked);
  }

  TEST_CASE("manual_acquire_and_release") {
    auto lock = dummy_lock();
    auto release = LockRelease(lock);
    REQUIRE(!lock.m_is_locked);
    release.acquire();
    REQUIRE(lock.m_is_locked);
    release.release();
    REQUIRE(!lock.m_is_locked);
  }

  TEST_CASE("double_release_is_idempotent") {
    auto lock = dummy_lock();
    auto release = LockRelease(lock);
    REQUIRE(!lock.m_is_locked);
    release.release();
    REQUIRE(!lock.m_is_locked);
  }

  TEST_CASE("move_constructor_transfers_state") {
    auto lock = dummy_lock();
    auto release1 = LockRelease(lock);
    REQUIRE(!lock.m_is_locked);
    auto release2 = LockRelease(std::move(release1));
    REQUIRE(!lock.m_is_locked);
    release2.acquire();
    REQUIRE(lock.m_is_locked);
  }

  TEST_CASE("release_function_returns_lock_release") {
    auto lock = dummy_lock();
    auto r = release(lock);
    REQUIRE(!lock.m_is_locked);
  }
}
