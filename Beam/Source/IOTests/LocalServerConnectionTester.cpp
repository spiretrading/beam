#include <doctest/doctest.h>
#include "Beam/IO/LocalServerConnection.hpp"
#include "Beam/IO/SharedBuffer.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::Routines;
using namespace boost;

TEST_SUITE("LocalServerConnection") {
  TEST_CASE("close_then_accept") {
    auto server = LocalServerConnection<SharedBuffer>();
    server.Close();
    REQUIRE_THROWS_AS(server.Accept(), EndOfFileException);
  }

  TEST_CASE("accept_pending_channel") {
  }

  TEST_CASE("accept_future_channel") {
  }

  TEST_CASE("future_accept_of_future_channel") {
  }

  TEST_CASE("open_client_before_open_server") {
  }

  TEST_CASE("open_client_then_close_server") {
    auto server = LocalServerConnection<SharedBuffer>();
    auto handled = false;
    auto clientTask = RoutineHandler(Spawn(
      [&] {
        auto client = LocalClientChannel("client", server);
      }));
    server.Accept();
  }
}
