#include <doctest/doctest.h>
#include "Beam/Pointers/Out.hpp"

using namespace Beam;

namespace {
  struct Dummy {
    int m_value;

    int get() const {
      return m_value;
    }
  };
}

TEST_SUITE("Out") {
  TEST_CASE("get_and_dereference_modifies_value") {
    auto value = int();
    auto handle = out(value);
    REQUIRE(handle.get() == &value);
    *handle = 123;
    REQUIRE(value == 123);
  }

  TEST_CASE("operator_arrow_accesses_members") {
    auto dummy = Dummy();
    dummy.m_value = 7;
    auto handle = out(dummy);
    REQUIRE(handle->get() == 7);
    (*handle).m_value = 8;
    REQUIRE(dummy.m_value == 8);
  }

  TEST_CASE("const_out_methods") {
    auto value = int();
    value = 1;
    auto handle = out(value);
    const auto const_handle = handle;
    REQUIRE(const_handle.get() == &value);
    REQUIRE((*const_handle) == 1);
  }

  TEST_CASE("out_from_out_returns_same_target") {
    auto dummy = Dummy();
    dummy.m_value = 9;
    auto outer = out(dummy);
    auto inner = out(outer);
    REQUIRE(inner.get() == outer.get());
    inner->m_value = 10;
    REQUIRE(dummy.m_value == 10);
  }
}
