#include <memory>
#include <optional>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Utilities/TypeTraits.hpp"

using namespace Beam;

namespace {
  template<typename T>
  struct SingleTemplate {};

  template<typename T, typename U>
  struct DoubleTemplate {};

  template<typename... Args>
  struct VariadicTemplate {};

  template<typename T>
  class ClassTemplate {};

  struct NonTemplate {};
}

TEST_SUITE("IsInstance") {
  TEST_CASE("single_template_parameter_positive") {
    REQUIRE(IsInstance<SingleTemplate<int>, SingleTemplate>);
    REQUIRE(IsInstance<SingleTemplate<double>, SingleTemplate>);
    REQUIRE(IsInstance<SingleTemplate<std::string>, SingleTemplate>);
    REQUIRE(IsInstance<SingleTemplate<NonTemplate>, SingleTemplate>);
  }

  TEST_CASE("single_template_parameter_negative") {
    REQUIRE(!IsInstance<int, SingleTemplate>);
    REQUIRE(!IsInstance<NonTemplate, SingleTemplate>);
    REQUIRE(!IsInstance<SingleTemplate<int>, DoubleTemplate>);
    REQUIRE(!IsInstance<DoubleTemplate<int, int>, SingleTemplate>);
  }

  TEST_CASE("double_template_parameter_positive") {
    REQUIRE(IsInstance<DoubleTemplate<int, double>, DoubleTemplate>);
    REQUIRE(IsInstance<DoubleTemplate<std::string, int>, DoubleTemplate>);
    REQUIRE(
      IsInstance<DoubleTemplate<NonTemplate, NonTemplate>, DoubleTemplate>);
  }

  TEST_CASE("double_template_parameter_negative") {
    REQUIRE(!IsInstance<int, DoubleTemplate>);
    REQUIRE(!IsInstance<NonTemplate, DoubleTemplate>);
    REQUIRE(!IsInstance<SingleTemplate<int>, DoubleTemplate>);
    REQUIRE(!IsInstance<DoubleTemplate<int, int>, SingleTemplate>);
  }

  TEST_CASE("variadic_template_parameter_positive") {
    REQUIRE(IsInstance<VariadicTemplate<>, VariadicTemplate>);
    REQUIRE(IsInstance<VariadicTemplate<int>, VariadicTemplate>);
    REQUIRE(IsInstance<VariadicTemplate<int, double>, VariadicTemplate>);
    REQUIRE(IsInstance<
      VariadicTemplate<int, double, std::string>, VariadicTemplate>);
  }

  TEST_CASE("variadic_template_parameter_negative") {
    REQUIRE(!IsInstance<int, VariadicTemplate>);
    REQUIRE(!IsInstance<NonTemplate, VariadicTemplate>);
    REQUIRE(!IsInstance<SingleTemplate<int>, VariadicTemplate>);
    REQUIRE(!IsInstance<DoubleTemplate<int, int>, VariadicTemplate>);
  }

  TEST_CASE("class_template_positive") {
    REQUIRE(IsInstance<ClassTemplate<int>, ClassTemplate>);
    REQUIRE(IsInstance<ClassTemplate<double>, ClassTemplate>);
    REQUIRE(IsInstance<ClassTemplate<NonTemplate>, ClassTemplate>);
  }

  TEST_CASE("class_template_negative") {
    REQUIRE(!IsInstance<int, ClassTemplate>);
    REQUIRE(!IsInstance<NonTemplate, ClassTemplate>);
    REQUIRE(!IsInstance<ClassTemplate<int>, SingleTemplate>);
  }

  TEST_CASE("standard_library_templates_positive") {
    REQUIRE(IsInstance<std::vector<int>, std::vector>);
    REQUIRE(IsInstance<std::unique_ptr<int>, std::unique_ptr>);
    REQUIRE(IsInstance<std::shared_ptr<double>, std::shared_ptr>);
    REQUIRE(IsInstance<std::optional<std::string>, std::optional>);
  }

  TEST_CASE("standard_library_templates_negative") {
    REQUIRE(!IsInstance<int, std::vector>);
    REQUIRE(!IsInstance<std::vector<int>, std::unique_ptr>);
    REQUIRE(!IsInstance<std::unique_ptr<int>, std::shared_ptr>);
    REQUIRE(!IsInstance<std::optional<int>, std::vector>);
  }

  TEST_CASE("nested_templates_positive") {
    REQUIRE(IsInstance<SingleTemplate<DoubleTemplate<int, int>>,
      SingleTemplate>);
    REQUIRE(IsInstance<std::vector<std::unique_ptr<int>>, std::vector>);
    REQUIRE(IsInstance<std::optional<std::vector<int>>, std::optional>);
  }

  TEST_CASE("const_volatile_qualified_positive") {
    REQUIRE(IsInstance<const SingleTemplate<int>, SingleTemplate>);
    REQUIRE(IsInstance<volatile SingleTemplate<int>, SingleTemplate>);
    REQUIRE(IsInstance<const volatile SingleTemplate<int>, SingleTemplate>);
  }

  TEST_CASE("reference_types_negative") {
    REQUIRE(!IsInstance<SingleTemplate<int>&, SingleTemplate>);
    REQUIRE(!IsInstance<const SingleTemplate<int>&, SingleTemplate>);
    REQUIRE(!IsInstance<SingleTemplate<int>&&, SingleTemplate>);
  }

  TEST_CASE("pointer_types_negative") {
    REQUIRE(!IsInstance<SingleTemplate<int>*, SingleTemplate>);
    REQUIRE(!IsInstance<const SingleTemplate<int>*, SingleTemplate>);
    REQUIRE(!IsInstance<SingleTemplate<int>* const, SingleTemplate>);
  }
}
