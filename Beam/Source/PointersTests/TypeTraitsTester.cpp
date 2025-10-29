#include <doctest/doctest.h>
#include "Beam/Utilities/TypeTraits.hpp"

using namespace Beam;

namespace {
  struct ConstructorSelector {
    int m_which = 0;

    ConstructorSelector() = default;

    ConstructorSelector(const ConstructorSelector&)
      : m_which(1) {}

    template<typename U,
      typename = disable_copy_constructor_t<ConstructorSelector, U>>
    ConstructorSelector(U&&)
      : m_which(2) {}
  };

  struct MultiParameterConstructorSelector {
    int m_which = 0;

    MultiParameterConstructorSelector() = default;

    MultiParameterConstructorSelector(const MultiParameterConstructorSelector&)
      : m_which(1) {}

    template<typename U, typename =
      disable_copy_constructor_t<MultiParameterConstructorSelector, U>>
    MultiParameterConstructorSelector(U&&, int = 0)
      : m_which(2) {}
  };
}

TEST_SUITE("TypeTraits") {
  TEST_CASE("template_constructor_disabled_for_same_type") {
    auto original = ConstructorSelector();
    auto copy = ConstructorSelector(original);
    REQUIRE(copy.m_which == 1);
  }

  TEST_CASE("template_constructor_enabled_for_other_types") {
    auto instance = ConstructorSelector(42);
    REQUIRE(instance.m_which == 2);
  }

  TEST_CASE("multi_parameter_template_constructor_disabled_for_same_type") {
    auto original = MultiParameterConstructorSelector();
    auto copy = MultiParameterConstructorSelector(original);
    REQUIRE(copy.m_which == 1);
  }

  TEST_CASE("multi_parameter_template_constructor_enabled_for_other_types") {
    auto instance = MultiParameterConstructorSelector(3);
    REQUIRE(instance.m_which == 2);
  }
}
