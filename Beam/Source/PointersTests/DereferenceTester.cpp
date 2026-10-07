#include <concepts>
#include <doctest/doctest.h>
#include "Beam/Pointers/Dereference.hpp"

using namespace Beam;

namespace {
  class Pointer {
    public:
      int& operator *();
      const int& operator *() const;
      int* operator ->();
      const int* operator ->() const;
  };
}

TEST_SUITE("Dereference") {
  TEST_CASE("traits") {
    static_assert(IsDereferenceable<int*>);
    static_assert(!IsDereferenceable<int>);
    static_assert(is_managed_pointer_v<std::unique_ptr<int>>);
    static_assert(is_managed_pointer_v<std::shared_ptr<int>>);
    static_assert(!is_managed_pointer_v<int*>);
    REQUIRE(true);
  }

  TEST_CASE("dereference_type_trait") {
    REQUIRE((std::is_same_v<dereference_t<int*>, int>));
    REQUIRE((std::is_same_v<dereference_t<std::unique_ptr<int>>, int>));
    REQUIRE((std::is_same_v<dereference_t<int>, int>));
    REQUIRE((std::same_as<dereference_t<const int*>, const int>));
    REQUIRE((std::same_as<dereference_t<volatile int*>, volatile int>));
    REQUIRE((std::same_as<dereference_t<const volatile int*>,
      const volatile int>));
    REQUIRE((std::same_as<dereference_t<int* const&>, int>));
    REQUIRE((std::same_as<dereference_t<const int* const&>, const int>));
    REQUIRE((std::same_as<dereference_t<int* const*>, int* const>));
    REQUIRE((std::same_as<dereference_t<std::unique_ptr<const int>>,
      const int>));
    REQUIRE((std::same_as<dereference_t<std::shared_ptr<const int>>,
      const int>));
    REQUIRE((std::same_as<dereference_t<const std::shared_ptr<int>&>, int>));
    REQUIRE((std::same_as<dereference_t<const std::unique_ptr<int>&>, int>));
    REQUIRE((std::same_as<dereference_t<Pointer>, int>));
    REQUIRE((std::same_as<dereference_t<const Pointer&>, const int>));
    REQUIRE((std::same_as<dereference_t<const int&>, int>));
  }

  TEST_CASE("fully_dereference_nested_unique_ptr") {
    auto number(42);
    auto inner = std::make_unique<int>(number);
    auto outer = std::make_unique<std::unique_ptr<int>>(std::move(inner));
    auto& reference = fully_dereference(outer);
    REQUIRE(reference == number);
    reference = 100;
    REQUIRE(*(*outer) == 100);
  }

  TEST_CASE("fully_dereference_nested_raw_pointers") {
    auto number(5);
    auto pointer = &number;
    auto pointer_to_pointer = &pointer;
    auto& reference = fully_dereference(pointer_to_pointer);
    REQUIRE(reference == number);
    reference = 7;
    REQUIRE(number == 7);
  }

  TEST_CASE("fully_dereference_shared_ptr") {
    auto number(10);
    auto inner_shared = std::make_shared<int>(number);
    auto outer_shared = std::make_shared<std::shared_ptr<int>>(inner_shared);
    auto& reference = fully_dereference(outer_shared);
    REQUIRE(reference == number);
    reference = 11;
    REQUIRE(*(*outer_shared) == 11);
  }

  TEST_CASE("fully_dereference_non_dereferenceable_preserves_reference") {
    auto number(123);
    auto& reference = fully_dereference(number);
    REQUIRE(&reference == &number);
    reference = 321;
    REQUIRE(number == 321);
  }
}
