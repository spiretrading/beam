#include <doctest/doctest.h>
#include "Beam/SignalHandling/ConnectionGroup.hpp"

using namespace Beam;
using namespace boost;
using namespace boost::signals2;

TEST_SUITE("ConnectionGroup") {
  TEST_CASE("add_and_disconnect_all") {
    auto group = ConnectionGroup();
    auto void_signal = signal<void ()>();
    auto count = 0;
    auto connection = void_signal.connect([&] {
      ++count;
    });
    group.add(connection);
    void_signal();
    REQUIRE(count == 1);
    group.disconnect();
    void_signal();
    REQUIRE(count == 1);
  }

  TEST_CASE("add_with_publisher_and_disconnect_publisher") {
    auto group = ConnectionGroup();
    auto signal1 = signal<void()>();
    auto signal2 = signal<void()>();
    auto count1 = 0;
    auto count2 = 0;
    auto publisher1 = 0;
    auto publisher2 = 0;
    group.add(&publisher1, signal1.connect([&] {
      ++count1;
    }));
    group.add(&publisher2, signal2.connect([&] {
      ++count2;
    }));
    signal1();
    signal2();
    REQUIRE(count1 == 1);
    REQUIRE(count2 == 1);
    group.disconnect(&publisher1);
    signal1();
    signal2();
    REQUIRE(count1 == 1);
    REQUIRE(count2 == 2);
  }

  TEST_CASE("destructor_disconnects") {
    auto void_signal = signal<void()>();
    auto count = 0;
    {
      auto group = ConnectionGroup();
      auto connection = void_signal.connect([&] {
        ++count;
      });
      group.add(connection);
      void_signal();
      REQUIRE(count == 1);
    }
    void_signal();
    REQUIRE(count == 1);
  }
}
