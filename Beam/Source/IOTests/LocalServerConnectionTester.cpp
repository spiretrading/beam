#include "Beam/IOTests/LocalServerConnectionTester.hpp"
#include "Beam/Routines/RoutineHandler.hpp"

using namespace Beam;
using namespace Beam::IO;
using namespace Beam::IO::Tests;
using namespace Beam::Routines;
using namespace boost;
using namespace std;

void LocalServerConnectionTester::setUp() {
  m_serverConnection.emplace();
}

void LocalServerConnectionTester::tearDown() {
  m_serverConnection = std::nullopt;
}

void LocalServerConnectionTester::TestAcceptThenClose() {
  ServerConnection server;
  bool handled = false;
  RoutineHandler task = Spawn(
    [&] {
      server.Open();
      server.Close();
      try {
        server.Accept();
      } catch(const EndOfFileException&) {
        handled = true;
      }
    });
  task.Wait();
  CPPUNIT_ASSERT(handled);
}

void LocalServerConnectionTester::TestAcceptPendingChannel() {
}

void LocalServerConnectionTester::TestAcceptFutureChannel() {
}

void LocalServerConnectionTester::TestFutureAcceptOfFutureChannel() {
}

void LocalServerConnectionTester::TestOpenClientBeforeOpenServer() {
}

void LocalServerConnectionTester::TestOpenClientThenCloseServer() {
  ServerConnection server;
  bool handled = false;
  bool isOpen = false;
  server.Open();
  RoutineHandler clientTask = Spawn(
    [&] {
      LocalServerConnectionTester::ClientChannel client("client",
        Ref(server));
      client.GetConnection().Open();
    });
  unique_ptr<LocalServerConnectionTester::ServerConnection::Channel> channel =
    server.Accept();
  clientTask.Wait();
}

void LocalServerConnectionTester::TestOpenClientAfterCloseServer() {
}
