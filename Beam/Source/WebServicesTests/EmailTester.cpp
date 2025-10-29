#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <doctest/doctest.h>
#include "Beam/WebServices/Email.hpp"

using namespace Beam;
using namespace boost::posix_time;

TEST_SUITE("Email") {
  TEST_CASE("construct_with_from") {
    auto email = Email("sender@example.com");
    REQUIRE(email.get_from().get_address() == "sender@example.com");
    REQUIRE(email.get_to().empty());
    REQUIRE(email.get_subject().empty());
    REQUIRE(email.get_bodies().empty());
    REQUIRE(!email.get_sender().is_initialized());
    REQUIRE(!email.get_date().is_not_a_date_time());
  }

  TEST_CASE("construct_with_from_and_to") {
    auto email = Email("sender@example.com", "recipient@example.com");
    REQUIRE(email.get_from().get_address() == "sender@example.com");
    REQUIRE(email.get_to().size() == 1);
    REQUIRE(email.get_to()[0].get_address() == "recipient@example.com");
  }

  TEST_CASE("setters") {
    auto email = Email("old@example.com");
    email.set_from("new@example.com");
    REQUIRE(email.get_from().get_address() == "new@example.com");
    auto date = time_from_string("2025-03-15 10:30:00");
    email.set_date(date);
    REQUIRE(email.get_date() == date);
    email.set_sender("actual@example.com");
    REQUIRE(email.get_sender().is_initialized());
    REQUIRE(email.get_sender()->get_address() == "actual@example.com");
    email.set_subject("Test Subject");
    REQUIRE(email.get_subject() == "Test Subject");
  }

  TEST_CASE("add_to") {
    auto email = Email("from@example.com");
    REQUIRE(email.get_to().empty());
    email.add_to("to1@example.com");
    REQUIRE(email.get_to().size() == 1);
    email.add_to("to2@example.com");
    REQUIRE(email.get_to().size() == 2);
    REQUIRE(email.get_to()[0].get_address() == "to1@example.com");
    REQUIRE(email.get_to()[1].get_address() == "to2@example.com");
  }

  TEST_CASE("add_body") {
    auto email = Email("from@example.com");
    REQUIRE(email.get_bodies().empty());
    email.add_body("Plain text message");
    REQUIRE(email.get_bodies().size() == 1);
    REQUIRE(email.get_bodies()[0].m_content_type == "text/plain");
    REQUIRE(email.get_bodies()[0].m_message == "Plain text message");
    email.add_body("text/html", "<p>HTML message</p>");
    REQUIRE(email.get_bodies().size() == 2);
    REQUIRE(email.get_bodies()[1].m_content_type == "text/html");
    REQUIRE(email.get_bodies()[1].m_message == "<p>HTML message</p>");
  }

  TEST_CASE("set_header") {
    auto email = Email("from@example.com");
    email.set_header("X-Custom", "value1");
    REQUIRE(email.get_additional_headers().size() == 2);
    auto header = email.find_header("X-Custom");
    REQUIRE(header.is_initialized());
    REQUIRE(*header == "value1");
    email.set_header("X-Custom", "value2");
    REQUIRE(email.get_additional_headers().size() == 2);
    header = email.find_header("X-Custom");
    REQUIRE(*header == "value2");
  }

  TEST_CASE("find_header") {
    auto email = Email("from@example.com");
    auto mime = email.find_header("Mime-Version");
    REQUIRE(mime.is_initialized());
    REQUIRE(*mime == "1.0");
    auto missing = email.find_header("X-Missing");
    REQUIRE(!missing.is_initialized());
    email.set_header("X-Test", "test-value");
    auto test = email.find_header("x-test");
    REQUIRE(test.is_initialized());
    REQUIRE(*test == "test-value");
  }

  TEST_CASE("stream_basic") {
    auto email = Email("sender@test.com");
    email.add_to("recipient@test.com");
    email.set_subject("Hello");
    email.add_body("Test message");
    auto ss = std::stringstream();
    ss << email;
    auto output = ss.str();
    REQUIRE(output.find("From: sender@test.com\r\n") != std::string::npos);
    REQUIRE(output.find("To: recipient@test.com\r\n") != std::string::npos);
    REQUIRE(output.find("Subject: Hello\r\n") != std::string::npos);
    REQUIRE(output.find("Content-Type: text/plain\r\n") !=
      std::string::npos);
    REQUIRE(output.find("Mime-Version: 1.0\r\n") != std::string::npos);
    REQUIRE(output.find("Test message") != std::string::npos);
    REQUIRE(output.find("\r\n.\r\n") != std::string::npos);
  }

  TEST_CASE("stream_with_display_name") {
    auto email = Email(EmailAddress("admin@test.com", "Administrator"));
    email.add_to(EmailAddress("user@test.com", "User Name"));
    auto ss = std::stringstream();
    ss << email;
    auto output = ss.str();
    REQUIRE(output.find("From: Administrator <admin@test.com>\r\n") !=
      std::string::npos);
    REQUIRE(output.find("To: User Name <user@test.com>\r\n") !=
      std::string::npos);
  }

  TEST_CASE("stream_with_sender") {
    auto email = Email("from@test.com");
    email.set_sender("sender@test.com");
    auto ss = std::stringstream();
    ss << email;
    auto output = ss.str();
    REQUIRE(output.find("Sender: sender@test.com\r\n") !=
      std::string::npos);
  }

  TEST_CASE("stream_multiple_recipients") {
    auto email = Email("from@test.com");
    email.add_to("to1@test.com");
    email.add_to("to2@test.com");
    email.add_to("to3@test.com");
    auto ss = std::stringstream();
    ss << email;
    auto output = ss.str();
    REQUIRE(output.find("To: to1@test.com, to2@test.com, to3@test.com\r\n") !=
      std::string::npos);
  }

  TEST_CASE("stream_multiple_bodies") {
    auto email = Email("from@test.com");
    email.add_body("text/plain", "Plain text");
    email.add_body("text/html", "<p>HTML</p>");
    auto ss = std::stringstream();
    ss << email;
    auto output = ss.str();
    REQUIRE(output.find("Content-Type: multipart/alternative; boundary=") !=
      std::string::npos);
    REQUIRE(output.find("Content-Type: text/plain\r\n") !=
      std::string::npos);
    REQUIRE(output.find("Content-Type: text/html\r\n") !=
      std::string::npos);
    REQUIRE(output.find("Plain text") != std::string::npos);
    REQUIRE(output.find("<p>HTML</p>") != std::string::npos);
  }

  TEST_CASE("stream_custom_headers") {
    auto email = Email("from@test.com");
    email.set_header("X-Priority", "1");
    email.set_header("X-Mailer", "Custom");
    auto ss = std::stringstream();
    ss << email;
    auto output = ss.str();
    REQUIRE(output.find("X-Priority: 1\r\n") != std::string::npos);
    REQUIRE(output.find("X-Mailer: Custom\r\n") != std::string::npos);
  }

  TEST_CASE("stream_empty_subject") {
    auto email = Email("from@test.com");
    auto ss = std::stringstream();
    ss << email;
    auto output = ss.str();
    REQUIRE(output.find("Subject:") == std::string::npos);
  }

  TEST_CASE("stream_no_recipients") {
    auto email = Email("from@test.com");
    auto ss = std::stringstream();
    ss << email;
    auto output = ss.str();
    REQUIRE(output.find("To:") == std::string::npos);
  }
}
