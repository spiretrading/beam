#include <boost/optional/optional_io.hpp>
#include <doctest/doctest.h>
#include "Beam/WebServices/NullWebSessionDataStore.hpp"
#include "Beam/WebServices/WebSession.hpp"
#include "Beam/WebServices/WebSessionStore.hpp"

using namespace Beam;

namespace {
  struct TestSession : WebSession {
    int m_data;

    TestSession(std::string id)
      : WebSession(std::move(id)),
        m_data(0) {}
  };

  class InMemoryWebSessionDataStore {
    public:
      template<typename Session>
      std::unique_ptr<Session> load(const std::string& id) {
        auto it = m_sessions.find(id);
        if(it == m_sessions.end()) {
          return nullptr;
        }
        auto session = std::make_unique<Session>(id);
        return session;
      }

      template<typename Session>
      void store(const Session& session) {
        m_sessions[session.get_id()] = true;
      }

      template<typename Session>
      void remove(const Session& session) {
        m_sessions.erase(session.get_id());
      }

      template<std::invocable<> F>
      decltype(auto) with_transaction(F&& transaction) {
        return transaction();
      }

      void close() {}

      bool has_session(const std::string& id) const {
        return m_sessions.find(id) != m_sessions.end();
      }

    private:
      std::unordered_map<std::string, bool> m_sessions;
  };
}

TEST_SUITE("WebSessionStore") {
  TEST_CASE("create_session") {
    auto store = WebSessionStore<TestSession>();
    auto session = store.create();
    REQUIRE(session);
    REQUIRE(!session->get_id().empty());
    REQUIRE(!session->is_expired());
    REQUIRE(session->m_data == 0);
  }

  TEST_CASE("create_multiple_unique_sessions") {
    auto store = WebSessionStore<TestSession>();
    auto session1 = store.create();
    auto session2 = store.create();
    auto session3 = store.create();
    REQUIRE(session1->get_id() != session2->get_id());
    REQUIRE(session2->get_id() != session3->get_id());
    REQUIRE(session1->get_id() != session3->get_id());
  }

  TEST_CASE("get_creates_session_when_no_cookie") {
    auto store = WebSessionStore<TestSession>();
    auto request = HttpRequest(Uri("http://localhost/test"));
    auto response = HttpResponse();
    auto session = store.get(request, out(response));
    REQUIRE(session);
    REQUIRE(!session->get_id().empty());
    auto cookie = response.get_cookie(
      WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME);
    REQUIRE(cookie);
    REQUIRE(cookie->get_name() ==
      WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME);
    REQUIRE(cookie->get_value() == session->get_id());
  }

  TEST_CASE("get_retrieves_existing_session") {
    auto store = WebSessionStore<TestSession>();
    auto request1 = HttpRequest(Uri("http://localhost/test"));
    auto response1 = HttpResponse();
    auto session1 = store.get(request1, out(response1));
    session1->m_data = 42;
    auto cookie = response1.get_cookie(
      WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME);
    REQUIRE(cookie);
    auto request2 = HttpRequest(Uri("http://localhost/test"));
    request2.add(Cookie(cookie->get_name(), cookie->get_value()));
    auto response2 = HttpResponse();
    auto session2 = store.get(request2, out(response2));
    REQUIRE(session2);
    REQUIRE(session2->get_id() == session1->get_id());
    REQUIRE(session2->m_data == 42);
  }

  TEST_CASE("find_returns_nullptr_when_no_cookie") {
    auto store = WebSessionStore<TestSession>();
    auto request = HttpRequest(Uri("http://localhost/test"));
    auto session = store.find(request);
    REQUIRE(!session);
  }

  TEST_CASE("find_returns_existing_session") {
    auto store = WebSessionStore<TestSession>();
    auto request1 = HttpRequest(Uri("http://localhost/test"));
    auto response1 = HttpResponse();
    auto session1 = store.get(request1, out(response1));
    session1->m_data = 99;
    auto cookie = response1.get_cookie(
      WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME);
    REQUIRE(cookie);
    auto request2 = HttpRequest(Uri("http://localhost/test"));
    request2.add(Cookie(cookie->get_name(), cookie->get_value()));
    auto session2 = store.find(request2);
    REQUIRE(session2);
    REQUIRE(session2->get_id() == session1->get_id());
    REQUIRE(session2->m_data == 99);
  }

  TEST_CASE("end_session") {
    auto store = WebSessionStore<TestSession>();
    auto session = store.create();
    auto session_id = session->get_id();
    REQUIRE(!session->is_expired());
    store.end(*session);
    REQUIRE(session->is_expired());
    auto request = HttpRequest(Uri("http://localhost/test"));
    request.add(Cookie(WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME,
      session_id));
    auto found_session = store.find(request);
    REQUIRE(!found_session);
  }

  TEST_CASE("custom_session_name") {
    auto config = WebSessionStoreConfig();
    config.m_session_name = "custom_session";
    auto store = WebSessionStore<TestSession>(config);
    auto request = HttpRequest(Uri("http://localhost/test"));
    auto response = HttpResponse();
    auto session = store.get(request, out(response));
    auto cookie = response.get_cookie("custom_session");
    REQUIRE(cookie);
    REQUIRE(cookie->get_name() == "custom_session");
    REQUIRE(cookie->get_value() == session->get_id());
  }

  TEST_CASE("custom_domain_and_path") {
    auto config = WebSessionStoreConfig();
    config.m_domain = "example.com";
    config.m_path = "/app";
    auto store = WebSessionStore<TestSession>(config);
    auto request = HttpRequest(Uri("http://localhost/test"));
    auto response = HttpResponse();
    auto session = store.get(request, out(response));
    auto cookie = response.get_cookie(
      WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME);
    REQUIRE(cookie);
    REQUIRE(cookie->get_domain() == "example.com");
    REQUIRE(cookie->get_path() == "/app");
  }

  TEST_CASE("persist_session") {
    auto data_store = InMemoryWebSessionDataStore();
    auto store =
      WebSessionStore<TestSession, InMemoryWebSessionDataStore*>(&data_store);
    auto session = store.create();
    auto response = HttpResponse();
    store.persist(*session, out(response));
    REQUIRE(data_store.has_session(session->get_id()));
    auto cookie = response.get_cookie(
      WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME);
    REQUIRE(cookie);
  }

  TEST_CASE("unpersist_session") {
    auto data_store = InMemoryWebSessionDataStore();
    auto store =
      WebSessionStore<TestSession, InMemoryWebSessionDataStore*>(&data_store);
    auto session = store.create();
    auto response1 = HttpResponse();
    store.persist(*session, out(response1));
    REQUIRE(data_store.has_session(session->get_id()));
    auto response2 = HttpResponse();
    store.unpersist(*session, out(response2));
    REQUIRE(!data_store.has_session(session->get_id()));
    auto cookie = response2.get_cookie(
      WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME);
    REQUIRE(cookie);
  }

  TEST_CASE("load_persistent_session") {
    auto data_store = InMemoryWebSessionDataStore();
    auto store =
      WebSessionStore<TestSession, InMemoryWebSessionDataStore*>(&data_store);
    auto session1 = store.create();
    auto response1 = HttpResponse();
    store.persist(*session1, out(response1));
    auto cookie = response1.get_cookie(
      WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME);
    REQUIRE(cookie);
    auto request = HttpRequest(Uri("http://localhost/test"));
    request.add(Cookie(cookie->get_name(), cookie->get_value()));
    auto response2 = HttpResponse();
    auto session2 = store.get(request, out(response2));
    REQUIRE(session2);
    REQUIRE(session2->get_id() == session1->get_id());
  }

  TEST_CASE("get_creates_new_session_when_persistent_not_found") {
    auto data_store = InMemoryWebSessionDataStore();
    auto store =
      WebSessionStore<TestSession, InMemoryWebSessionDataStore*>(&data_store);
    auto request = HttpRequest(Uri("http://localhost/test"));
    request.add(Cookie(WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME,
      "nonexistent_id"));
    auto response = HttpResponse();
    auto session = store.get(request, out(response));
    REQUIRE(session);
    REQUIRE(session->get_id() != "nonexistent_id");
  }

  TEST_CASE("set_web_session_id_cookie") {
    auto store = WebSessionStore<TestSession>();
    auto session = store.create();
    auto response = HttpResponse();
    store.set_web_session_id_cookie(*session, out(response));
    auto cookie = response.get_cookie(
      WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME);
    REQUIRE(cookie);
    REQUIRE(cookie->get_value() == session->get_id());
    REQUIRE(cookie->get_path() == "/");
  }

  TEST_CASE("multiple_sessions_coexist") {
    auto store = WebSessionStore<TestSession>();
    auto session1 = store.create();
    auto session2 = store.create();
    auto session3 = store.create();
    session1->m_data = 10;
    session2->m_data = 20;
    session3->m_data = 30;
    auto request1 = HttpRequest(Uri("http://localhost/test"));
    request1.add(Cookie(WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME,
      session1->get_id()));
    auto found1 = store.find(request1);
    REQUIRE(found1);
    REQUIRE(found1->m_data == 10);
    auto request2 = HttpRequest(Uri("http://localhost/test"));
    request2.add(Cookie(WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME,
      session2->get_id()));
    auto found2 = store.find(request2);
    REQUIRE(found2);
    REQUIRE(found2->m_data == 20);
    auto request3 = HttpRequest(Uri("http://localhost/test"));
    request3.add(Cookie(WebSessionStoreConfig::DEFAULT_WEB_SESSION_NAME,
      session3->get_id()));
    auto found3 = store.find(request3);
    REQUIRE(found3);
    REQUIRE(found3->m_data == 30);
  }
}
