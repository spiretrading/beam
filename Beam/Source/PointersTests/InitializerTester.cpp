#include <memory>
#include <doctest/doctest.h>
#include "Beam/Pointers/Initializer.hpp"

using namespace Beam;

namespace {
  struct MoveOnlyArg {
    std::unique_ptr<int> m_value;

    explicit MoveOnlyArg(int v)
      : m_value(std::make_unique<int>(v)) {}

    MoveOnlyArg(const MoveOnlyArg&) = delete;
    MoveOnlyArg(MoveOnlyArg&&) = default;
  };

  struct CopyOnlyArg {
    int m_value;

    explicit CopyOnlyArg(int v)
      : m_value(v) {}

    CopyOnlyArg(const CopyOnlyArg&) = default;
    CopyOnlyArg(CopyOnlyArg&&) = delete;
  };

  struct WidgetFromMoveOnly {
    std::unique_ptr<int> m_value;

    explicit WidgetFromMoveOnly(MoveOnlyArg arg)
      : m_value(std::move(arg.m_value)) {}
  };

  struct WidgetFromCopyOnly {
    CopyOnlyArg m_arg;

    explicit WidgetFromCopyOnly(CopyOnlyArg arg)
      : m_arg(arg) {}
  };

  struct Tracker {
    inline static int copies = 0;
    inline static int moves = 0;

    Tracker() = default;
    Tracker(const Tracker&) { ++copies; }
    Tracker(Tracker&&) noexcept { ++moves; }
  };

  struct WidgetFromTracker {
    Tracker m_arg;

    explicit WidgetFromTracker(Tracker arg)
      : m_arg(std::move(arg)) {}
  };
}

TEST_SUITE("Initializer") {
  TEST_CASE("move_only_type") {
    auto instance = make<WidgetFromMoveOnly>(init(MoveOnlyArg(42)));
    REQUIRE(instance.m_value);
    REQUIRE(*instance.m_value == 42);
  }

  TEST_CASE("copy_only_type") {
    auto arg = CopyOnlyArg(7);
    auto instance = make<WidgetFromCopyOnly>(init(arg));
    REQUIRE(instance.m_arg.m_value == 7);
  }

  TEST_CASE("fundamental_type") {
    auto original = 2;
    auto value = make<int>(init(original));
    REQUIRE(value == 2);
  }

  TEST_CASE("literal_type") {
    auto value = make<int>(init(123));
    REQUIRE(value == 123);
  }

  TEST_CASE("initializer_allows_moving_unique_ptr_through") {
    struct Holder {
      std::unique_ptr<int> m_ptr;
      explicit Holder(std::unique_ptr<int> p)
        : m_ptr(std::move(p)) {}
    };
    auto instance = make<Holder>(init(std::make_unique<int>(99)));
    REQUIRE(instance.m_ptr);
    REQUIRE(*instance.m_ptr == 99);
  }

  TEST_CASE("temporary_argument_moves_and_lvalue_copies_for_tracker") {
    Tracker::copies = 0;
    Tracker::moves = 0;
    auto instance_temp = make<WidgetFromTracker>(init(Tracker()));
    REQUIRE(Tracker::moves >= 1);
    REQUIRE(Tracker::copies == 0);
    Tracker::copies = 0;
    Tracker::moves = 0;
    auto tracker = Tracker();
    auto instance_lvalue = make<WidgetFromTracker>(init(tracker));
    REQUIRE(Tracker::copies >= 1);
  }

  TEST_CASE("initializer_type_deduction_for_lvalue_is_reference") {
    auto x = 5;
    using init_t = decltype(init(x));
    static_assert(std::is_same_v<init_t, Initializer<int&>>,
      "Initializer should deduce reference type for lvalue arguments.");
  }
}
