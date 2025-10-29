#include <doctest/doctest.h>
#include "Beam/Pointers/LocalPtr.hpp"

using namespace Beam;

namespace {
  struct Dummy {
    int m_value;
    int get() const {
      return m_value;
    }
  };

  struct MoveOnly {
    std::unique_ptr<int> m_ptr;
    explicit MoveOnly(int v)
      : m_ptr(std::make_unique<int>(v)) {}
    MoveOnly(MoveOnly&&) noexcept = default;
    MoveOnly(const MoveOnly&) = delete;
  };

  struct Copyable {
    int m_value;
    explicit Copyable(int v)
      : m_value(v) {}
  };
}

TEST_SUITE("LocalPtr") {
  TEST_CASE("construct_and_access_primitives") {
    auto lp = LocalPtr<int>(42);
    REQUIRE(static_cast<bool>(lp));
    REQUIRE(*lp == 42);
    REQUIRE(*lp == *lp);
    REQUIRE(lp.get() == &*lp);
  }

  TEST_CASE("operator_arrow_and_get_member_access") {
    auto lp = LocalPtr<Dummy>(Dummy{7});
    REQUIRE(lp->get() == 7);
    (*lp).m_value = 8;
    REQUIRE(lp->m_value == 8);
    REQUIRE(lp.get()->m_value == 8);
  }

  TEST_CASE("capture_ptr_returns_pointer_for_lvalue_and_localptr_for_rvalue") {
    auto x = 3;
    auto p = capture_ptr<int&>(x);
    REQUIRE(std::is_pointer_v<decltype(p)>);
    REQUIRE(*p == 3);
    auto r = capture_ptr<int&&>(5);
    REQUIRE(std::is_same_v<decltype(r), LocalPtr<int>>);
    REQUIRE(*r == 5);
  }

  TEST_CASE("move_only_type_moves_into_localptr_and_moves_between_localptrs") {
    auto lp = LocalPtr<MoveOnly>(MoveOnly(99));
    REQUIRE(lp->m_ptr);
    REQUIRE(*lp->m_ptr == 99);
    auto lp2 = std::move(lp);
    REQUIRE(lp2->m_ptr);
    REQUIRE(*lp2->m_ptr == 99);
  }

  TEST_CASE("copyable_type_is_copiable_via_localptr_copy") {
    auto lp1 = LocalPtr<Copyable>(Copyable(11));
    auto lp2 = lp1;
    REQUIRE(lp1->m_value == 11);
    REQUIRE(lp2->m_value == 11);
    lp2->m_value = 12;
    REQUIRE(lp1->m_value == 11);
    REQUIRE(lp2->m_value == 12);
  }

  TEST_CASE("initializer_constructor_constructs_from_initializer") {
    auto lp = LocalPtr<int>(init(13));
    REQUIRE(*lp == 13);
    auto init_move = init(MoveOnly(21));
    auto lp_move = LocalPtr<MoveOnly>(init(MoveOnly(21)));
    REQUIRE(lp_move->m_ptr);
    REQUIRE(*lp_move->m_ptr == 21);
  }

  TEST_CASE("local_ptr_t_selects_localptr_for_non_dereferenceable") {
    REQUIRE(std::is_same_v<local_ptr_t<int>, LocalPtr<int>>);
    REQUIRE(std::is_same_v<local_ptr_t<int*>, int*>);
  }
}
