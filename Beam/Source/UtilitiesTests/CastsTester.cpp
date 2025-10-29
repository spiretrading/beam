#include <memory>
#include <doctest/doctest.h>
#include "Beam/Utilities/Casts.hpp"

using namespace Beam;

namespace {
  struct Base {
    int m_base_value = 10;

    virtual ~Base() = default;
  };

  struct Derived : Base {
    int m_derived_value = 20;
  };
}

TEST_SUITE("Casts") {
  TEST_CASE("static_pointer_cast_derived_to_base") {
    auto derived = std::make_unique<Derived>();
    derived->m_base_value = 100;
    derived->m_derived_value = 200;
    auto base = static_pointer_cast<Base>(std::move(derived));
    REQUIRE(!derived.get());
    REQUIRE(base->m_base_value == 100);
  }

  TEST_CASE("static_pointer_cast_preserves_value") {
    auto derived = std::make_unique<Derived>();
    auto* original_ptr = derived.get();
    auto base = static_pointer_cast<Base>(std::move(derived));
    REQUIRE(base.get() == original_ptr);
    REQUIRE(base->m_base_value == 10);
  }
}
