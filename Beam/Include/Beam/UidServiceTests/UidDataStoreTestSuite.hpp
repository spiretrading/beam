#ifndef BEAM_UID_DATA_STORE_TEST_SUITE_HPP
#define BEAM_UID_DATA_STORE_TEST_SUITE_HPP
#include <doctest/doctest.h>
#include "Beam/UidService/UidDataStore.hpp"

namespace Beam::Tests {
  TEST_CASE_TEMPLATE_DEFINE("UidDataStore", T, UidDataStoreTestSuite) {
    using namespace Beam;
    auto data_store = T()();

    SUBCASE("initial_uid") {
      auto uid = data_store.get_next_uid();
      REQUIRE(uid == 1);
    }

    SUBCASE("reserve_single") {
      auto uid = data_store.reserve(1);
      REQUIRE(uid == 1);
      REQUIRE(data_store.get_next_uid() == 2);
    }

    SUBCASE("reserve_multiple") {
      auto uid = data_store.reserve(5);
      REQUIRE(uid == 1);
      REQUIRE(data_store.get_next_uid() == 6);
    }

    SUBCASE("reserve_zero") {
      auto uid = data_store.reserve(0);
      REQUIRE(uid == 1);
      REQUIRE(data_store.get_next_uid() == 1);
    }

    SUBCASE("sequential_reserves") {
      auto first = data_store.reserve(3);
      auto second = data_store.reserve(2);
      auto third = data_store.reserve(1);
      REQUIRE(first == 1);
      REQUIRE(second == 4);
      REQUIRE(third == 6);
      REQUIRE(data_store.get_next_uid() == 7);
    }

    SUBCASE("transaction_isolation") {
      auto uid_inside = std::uint64_t(0);
      data_store.with_transaction([&] {
        uid_inside = data_store.get_next_uid();
        data_store.reserve(5);
      });
      REQUIRE(uid_inside == 1);
      REQUIRE(data_store.get_next_uid() == 6);
    }

    SUBCASE("close") {
      data_store.reserve(5);
      data_store.close();
      REQUIRE_THROWS(data_store.get_next_uid());
    }

    SUBCASE("multiple_close") {
      data_store.close();
      data_store.close();
      REQUIRE_THROWS(data_store.get_next_uid());
    }
  }
}

#endif
