#include <doctest/doctest.h>
#include <Viper/Sqlite3/Connection.hpp>
#include "Beam/Sql/SqlConnection.hpp"
#include "Beam/WebServices/SqlWebSessionDataStore.hpp"
#include "Beam/WebServices/WebSession.hpp"

using namespace Beam;
using namespace Viper;
using namespace Viper::Sqlite3;

namespace {
  using TestSqlWebSessionDataStore =
    SqlWebSessionDataStore<SqlConnection<Sqlite3::Connection>>;

  auto make_test_data_store() {
    return TestSqlWebSessionDataStore(
      std::make_unique<SqlConnection<Sqlite3::Connection>>(
        "file::memory:?cache=shared"));
  }

  struct TestSession : WebSession {
    int m_data;
    std::string m_username;

    TestSession()
      : TestSession("") {}

    TestSession(std::string id)
      : WebSession(std::move(id)),
        m_data(0) {}

    template<IsShuttle S>
    void shuttle(S& shuttle, unsigned int version) {
      WebSession::shuttle(shuttle, version);
      shuttle.shuttle("data", m_data);
      shuttle.shuttle("username", m_username);
    }
  };
}

TEST_SUITE("SqlWebSessionDataStore") {
  TEST_CASE("store_and_load_session") {
    auto data_store = make_test_data_store();
    auto session = TestSession("test_session_id_123");
    session.m_data = 42;
    session.m_username = "testuser";
    data_store.store(session);
    auto loaded_session = data_store.load<TestSession>("test_session_id_123");
    REQUIRE(loaded_session);
    REQUIRE(loaded_session->get_id() == "test_session_id_123");
    REQUIRE(loaded_session->m_data == 42);
    REQUIRE(loaded_session->m_username == "testuser");
    REQUIRE(!loaded_session->is_expired());
  }

  TEST_CASE("load_nonexistent_session_returns_nullptr") {
    auto data_store = make_test_data_store();
    REQUIRE_THROWS_AS(data_store.load<TestSession>("nonexistent_id"),
      WebSessionDataStoreException);
  }

  TEST_CASE("update_existing_session") {
    auto data_store = make_test_data_store();
    auto session = TestSession("update_test_id");
    session.m_data = 100;
    session.m_username = "original_user";
    data_store.store(session);
    session.m_data = 200;
    session.m_username = "updated_user";
    data_store.store(session);
    auto loaded_session = data_store.load<TestSession>("update_test_id");
    REQUIRE(loaded_session);
    REQUIRE(loaded_session->m_data == 200);
    REQUIRE(loaded_session->m_username == "updated_user");
  }

  TEST_CASE("remove_session") {
    auto data_store = make_test_data_store();
    auto session = TestSession("remove_test_id");
    session.m_data = 99;
    data_store.store(session);
    auto loaded_before = data_store.load<TestSession>("remove_test_id");
    REQUIRE(loaded_before);
    data_store.remove(session);
    REQUIRE_THROWS_AS(data_store.load<TestSession>("remove_test_id"),
      WebSessionDataStoreException);
  }

  TEST_CASE("store_multiple_sessions") {
    auto data_store = make_test_data_store();
    auto session1 = TestSession("multi_session_1");
    session1.m_data = 10;
    session1.m_username = "user1";
    auto session2 = TestSession("multi_session_2");
    session2.m_data = 20;
    session2.m_username = "user2";
    auto session3 = TestSession("multi_session_3");
    session3.m_data = 30;
    session3.m_username = "user3";
    data_store.store(session1);
    data_store.store(session2);
    data_store.store(session3);
    auto loaded1 = data_store.load<TestSession>("multi_session_1");
    REQUIRE(loaded1);
    REQUIRE(loaded1->m_data == 10);
    auto loaded2 = data_store.load<TestSession>("multi_session_2");
    REQUIRE(loaded2);
    REQUIRE(loaded2->m_data == 20);
    auto loaded3 = data_store.load<TestSession>("multi_session_3");
    REQUIRE(loaded3);
    REQUIRE(loaded3->m_data == 30);
  }

  TEST_CASE("session_expires_flag_persists") {
    auto data_store = make_test_data_store();
    auto session = TestSession("expired_test_id");
    session.m_data = 55;
    session.set_expired();
    data_store.store(session);
    auto loaded_session = data_store.load<TestSession>("expired_test_id");
    REQUIRE(loaded_session);
    REQUIRE(loaded_session->is_expired());
    REQUIRE(loaded_session->m_data == 55);
  }

  TEST_CASE("with_transaction_void_return") {
    auto data_store = make_test_data_store();
    auto session = TestSession("transaction_test_id");
    session.m_data = 77;
    data_store.with_transaction([&] {
      data_store.store(session);
    });
    auto loaded_session = data_store.load<TestSession>("transaction_test_id");
    REQUIRE(loaded_session);
    REQUIRE(loaded_session->m_data == 77);
  }

  TEST_CASE("with_transaction_non_void_return") {
    auto data_store = make_test_data_store();
    auto result = data_store.with_transaction([&] {
      return 42;
    });
    REQUIRE(result == 42);
  }

  TEST_CASE("empty_session_data") {
    auto data_store = make_test_data_store();
    auto session = TestSession("empty_data_id");
    data_store.store(session);
    auto loaded_session = data_store.load<TestSession>("empty_data_id");
    REQUIRE(loaded_session);
    REQUIRE(loaded_session->m_data == 0);
    REQUIRE(loaded_session->m_username.empty());
  }
}
