#include <string>
#include <doctest/doctest.h>
#include <boost/optional/optional_io.hpp>
#include "Beam/Queries/PagedQuery.hpp"

using namespace Beam;
using namespace Beam::Queries;

TEST_SUITE("IndexedQuery") {
  TEST_CASE("default_constructor") {
    auto query = PagedQuery<int, std::string>();
    REQUIRE(query.GetIndex() == 0);
    REQUIRE(!query.GetAnchor());
  }

  TEST_CASE("set_anchor") {
    auto intStringQuery = PagedQuery<int, std::string>();
    intStringQuery.SetAnchor("Hello world");
    REQUIRE(intStringQuery.GetAnchor() == std::string("Hello world"));
  }
}
