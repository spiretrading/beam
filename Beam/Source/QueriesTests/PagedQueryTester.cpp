#include <string>
#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Beam/Queries/PagedQuery.hpp"

using namespace Beam;

TEST_SUITE("IndexedQuery") {
  TEST_CASE("default_constructor") {
    auto query = PagedQuery<int, std::string>();
    REQUIRE(query.get_index() == 0);
    REQUIRE(!query.get_anchor());
  }

  TEST_CASE("set_anchor") {
    auto query = PagedQuery<int, std::string>();
    query.set_anchor("Hello world");
    REQUIRE(query.get_anchor() == std::string("Hello world"));
  }
}
