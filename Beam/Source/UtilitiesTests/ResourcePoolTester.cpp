#include <memory>
#include <string>
#include <vector>
#include <doctest/doctest.h>
#include "Beam/Utilities/ResourcePool.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::posix_time;

namespace {
  struct Resource {
    static inline auto construction_count = 0;
    static inline auto destruction_count = 0;
    int value = 0;

    static void reset_counters() {
      construction_count = 0;
      destruction_count = 0;
    }

    Resource() {
      ++construction_count;
    }

    Resource(int v)
        : value(v) {
      ++construction_count;
    }

    ~Resource() {
      ++destruction_count;
    }
  };
}

TEST_SUITE("ResourcePool") {
  TEST_CASE("construct_with_minimum_resources") {
    Resource::reset_counters();
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(); }, 3);
    REQUIRE(Resource::construction_count == 3);
  }

  TEST_CASE("construct_with_zero_minimum_uses_one") {
    Resource::reset_counters();
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(); }, 0);
    REQUIRE(Resource::construction_count == 1);
  }

  TEST_CASE("construct_with_max_less_than_min_adjusts_max") {
    Resource::reset_counters();
    auto pool = ResourcePool(
      seconds(1), [] { return std::make_unique<Resource>(); }, 5, 3);
    REQUIRE(Resource::construction_count == 5);
    auto resource1 = pool.load();
    auto resource2 = pool.load();
    auto resource3 = pool.load();
    auto resource4 = pool.load();
    auto resource5 = pool.load();
    REQUIRE(Resource::construction_count == 5);
  }

  TEST_CASE("load_returns_resource") {
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(42); });
    auto resource = pool.load();
    REQUIRE(resource->value == 42);
  }

  TEST_CASE("load_reuses_returned_resource") {
    Resource::reset_counters();
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(); }, 1);
    REQUIRE(Resource::construction_count == 1);
    {
      auto resource = pool.load();
      resource->value = 99;
    }
    auto resource = pool.load();
    REQUIRE(resource->value == 99);
    REQUIRE(Resource::construction_count == 1);
  }

  TEST_CASE("try_load_returns_resource_when_available") {
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(123); });
    auto resource = pool.try_load();
    REQUIRE(resource.has_value());
    REQUIRE((*resource)->value == 123);
  }

  TEST_CASE("try_load_returns_empty_when_unavailable") {
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(); }, 1);
    auto resource1 = pool.load();
    auto resource2 = pool.try_load();
    REQUIRE_FALSE(resource2.has_value());
  }

  TEST_CASE("try_load_succeeds_after_resource_returned") {
    auto pool = ResourcePool(
      seconds(1), [] { return std::make_unique<Resource>(55); }, 1);
    {
      auto resource1 = pool.load();
    }
    auto resource2 = pool.try_load();
    REQUIRE(resource2.has_value());
    REQUIRE((*resource2)->value == 55);
  }

  TEST_CASE("multiple_resources_can_be_acquired") {
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(); }, 3);
    auto resource1 = pool.load();
    auto resource2 = pool.load();
    auto resource3 = pool.load();
    resource1->value = 1;
    resource2->value = 2;
    resource3->value = 3;
    REQUIRE(resource1->value == 1);
    REQUIRE(resource2->value == 2);
    REQUIRE(resource3->value == 3);
  }

  TEST_CASE("resources_returned_in_fifo_order") {
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(); }, 3);
    auto resource1 = make_optional(pool.load());
    auto resource2 = make_optional(pool.load());
    auto resource3 = make_optional(pool.load());
    (*resource1)->value = 10;
    (*resource2)->value = 20;
    (*resource3)->value = 30;
    auto pointer1 = &(**resource1);
    auto pointer2 = &(**resource2);
    auto pointer3 = &(**resource3);
    resource1 = none;
    resource2 = none;
    resource3 = none;
    auto new_resource1 = pool.load();
    auto new_resource2 = pool.load();
    auto new_resource3 = pool.load();
    REQUIRE(&(*new_resource1) == pointer1);
    REQUIRE(&(*new_resource2) == pointer2);
    REQUIRE(&(*new_resource3) == pointer3);
  }

  TEST_CASE("reset_destroys_and_rebuilds_resources") {
    Resource::reset_counters();
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(); }, 2);
    REQUIRE(Resource::construction_count == 2);
    REQUIRE(Resource::destruction_count == 0);
    pool.reset();
    REQUIRE(Resource::construction_count == 4);
    REQUIRE(Resource::destruction_count == 2);
  }

  TEST_CASE("builder_with_state") {
    auto counter = 0;
    auto builder = [&] {
      return std::make_unique<Resource>(++counter);
    };
    auto pool = ResourcePool(seconds(1), builder, 3);
    auto resource1 = pool.load();
    auto resource2 = pool.load();
    auto resource3 = pool.load();
    auto values =
      std::vector{resource1->value, resource2->value, resource3->value};
    std::sort(values.begin(), values.end());
    REQUIRE(values[0] == 1);
    REQUIRE(values[1] == 2);
    REQUIRE(values[2] == 3);
  }
}

TEST_SUITE("ScopedResource") {
  TEST_CASE("dereference_operator") {
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(77); });
    auto resource = pool.load();
    REQUIRE((*resource).value == 77);
  }

  TEST_CASE("arrow_operator") {
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(88); });
    auto resource = pool.load();
    REQUIRE(resource->value == 88);
  }

  TEST_CASE("move_constructor") {
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(11); });
    auto resource1 = pool.load();
    auto pointer = &(*resource1);
    auto resource2 = std::move(resource1);
    REQUIRE(&(*resource2) == pointer);
    REQUIRE(resource2->value == 11);
  }

  TEST_CASE("moved_from_resource_does_not_return_to_pool") {
    Resource::reset_counters();
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(); }, 1);
    auto resource2 = [&] {
      auto resource1 = pool.load();
      return resource1;
    }();
    REQUIRE(Resource::destruction_count == 0);
  }

  TEST_CASE("destructor_returns_resource_to_pool") {
    auto pool = ResourcePool(
      seconds(1), [] { return std::make_unique<Resource>(66); }, 1);
    {
      auto resource = pool.load();
      resource->value = 100;
    }
    auto resource = pool.load();
    REQUIRE(resource->value == 100);
  }

  TEST_CASE("multiple_scoped_resources_with_different_values") {
    auto pool =
      ResourcePool(seconds(1), [] { return std::make_unique<Resource>(); }, 2);
    auto resource1 = pool.load();
    auto resource2 = pool.load();
    resource1->value = 111;
    resource2->value = 222;
    REQUIRE(resource1->value == 111);
    REQUIRE(resource2->value == 222);
  }
}
