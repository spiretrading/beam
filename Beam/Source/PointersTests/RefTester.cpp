#include <utility>
#include <doctest/doctest.h>
#include "Beam/Pointers/Ref.hpp"

using namespace Beam;

namespace {
  struct Base {
    int value = 0;
    int get() const { return value; }
  };

  struct Derived : Base {};

  struct Unrelated {};
}

TEST_SUITE("Ref") {
  TEST_CASE("construct_and_access") {
    auto x = 5;
    auto r = Ref(x);
    REQUIRE(r.get() == &x);
    REQUIRE(*r == 5);
    *r = 6;
    REQUIRE(x == 6);
    auto obj = Base();
    obj.value = 7;
    auto robj = Ref(obj);
    REQUIRE(robj->get() == 7);
    (*robj).value = 8;
    REQUIRE(obj.value == 8);
  }

  TEST_CASE("copy_and_move_semantics") {
    auto obj = Base();
    obj.value = 10;
    auto r1 = Ref(obj);
    auto r2 = r1;
    REQUIRE(r1.get() == r2.get());
    REQUIRE(r1.get() == &obj);
    auto r3 = std::move(r1);
    REQUIRE(r3.get() == &obj);
    REQUIRE(!r1.get());
    r2 = std::move(r3);
    REQUIRE(r2.get() == &obj);
    REQUIRE(!r3.get());
    auto before = r2.get();
    r2 = std::move(r2);
    REQUIRE(r2.get() == before);
  }

  TEST_CASE("converting_constructor_polymorphism") {
    auto d = Derived();
    d.value = 42;
    auto rd = Ref(d);
    static_assert(std::is_constructible_v<Ref<Base>, Ref<Derived>>);
    auto rb = Ref<Base>(rd);
    REQUIRE(rb.get() == &d);
    REQUIRE(rb->get() == 42);
    static_assert(!std::is_constructible_v<Ref<Base>, Ref<Unrelated>>);
  }

  TEST_CASE("get_and_dereference_consistency") {
    auto obj = Derived();
    obj.value = 3;
    auto r = Ref(obj);
    REQUIRE(r.get() == &obj);
    REQUIRE(&*r == r.get());
    REQUIRE(r->get() == 3);
  }
}
