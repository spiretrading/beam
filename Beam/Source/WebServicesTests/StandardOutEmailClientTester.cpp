#include <doctest/doctest.h>
#include "Beam/WebServices/StandardOutEmailClient.hpp"

using namespace Beam;

namespace {
  struct CoutRedirect {
    std::stringstream m_buffer;
    std::streambuf* m_old;

    CoutRedirect()
      : m_old(std::cout.rdbuf(m_buffer.rdbuf())) {}

    ~CoutRedirect() {
      std::cout.rdbuf(m_old);
    }

    std::string get_output() const {
      return m_buffer.str();
    }
  };
}

TEST_SUITE("StandardOutEmailClient") {
  TEST_CASE("send_basic_email") {
    auto redirect = CoutRedirect();
    auto client = StandardOutEmailClient();
    auto email = Email(EmailAddress("sender@example.com"),
      EmailAddress("recipient@example.com"));
    email.set_subject("Test Subject");
    email.add_body("Test Body");
    client.send(email);
    auto output = redirect.get_output();
    REQUIRE(output.find("From: sender@example.com") != std::string::npos);
    REQUIRE(output.find("To: recipient@example.com") != std::string::npos);
    REQUIRE(output.find("Subject: Test Subject") != std::string::npos);
    REQUIRE(output.find("Test Body") != std::string::npos);
  }

  TEST_CASE("send_multiple_emails") {
    auto redirect = CoutRedirect();
    auto client = StandardOutEmailClient();
    auto email1 = Email(EmailAddress("sender1@example.com"),
      EmailAddress("recipient1@example.com"));
    email1.set_subject("First Email");
    email1.add_body("First Body");
    auto email2 = Email(EmailAddress("sender2@example.com"),
      EmailAddress("recipient2@example.com"));
    email2.set_subject("Second Email");
    email2.add_body("Second Body");
    client.send(email1);
    client.send(email2);
    auto output = redirect.get_output();
    REQUIRE(output.find("From: sender1@example.com") != std::string::npos);
    REQUIRE(output.find("Subject: First Email") != std::string::npos);
    REQUIRE(output.find("First Body") != std::string::npos);
    REQUIRE(output.find("From: sender2@example.com") != std::string::npos);
    REQUIRE(output.find("Subject: Second Email") != std::string::npos);
    REQUIRE(output.find("Second Body") != std::string::npos);
  }

  TEST_CASE("send_after_close") {
    auto redirect = CoutRedirect();
    auto client = StandardOutEmailClient();
    auto email = Email(EmailAddress("sender@example.com"),
      EmailAddress("recipient@example.com"));
    email.set_subject("Test");
    email.add_body("Body");
    client.close();
    REQUIRE_THROWS_AS(client.send(email), EndOfFileException);
    auto output = redirect.get_output();
    REQUIRE(output.empty());
  }

  TEST_CASE("send_with_multiple_recipients") {
    auto redirect = CoutRedirect();
    auto client = StandardOutEmailClient();
    auto email = Email(EmailAddress("sender@example.com"));
    email.add_to(EmailAddress("recipient1@example.com"));
    email.add_to(EmailAddress("recipient2@example.com"));
    email.add_to(EmailAddress("recipient3@example.com"));
    email.set_subject("Multiple Recipients");
    email.add_body("Message to all");
    client.send(email);
    auto output = redirect.get_output();
    REQUIRE(output.find("From: sender@example.com") != std::string::npos);
    REQUIRE(output.find("To: recipient1@example.com, recipient2@example.com, "
      "recipient3@example.com") != std::string::npos);
    REQUIRE(output.find("Subject: Multiple Recipients") != std::string::npos);
    REQUIRE(output.find("Message to all") != std::string::npos);
  }

  TEST_CASE("destructor_closes") {
    auto redirect = CoutRedirect();
    {
      auto client = StandardOutEmailClient();
      auto email = Email(EmailAddress("sender@example.com"),
        EmailAddress("recipient@example.com"));
      email.set_subject("Test");
      email.add_body("Body");
      client.send(email);
    }
    auto output = redirect.get_output();
    REQUIRE(output.find("From: sender@example.com") != std::string::npos);
  }

  TEST_CASE("email_with_sender") {
    auto redirect = CoutRedirect();
    auto client = StandardOutEmailClient();
    auto email =
      Email(EmailAddress("from@example.com"), EmailAddress("to@example.com"));
    email.set_sender(EmailAddress("sender@example.com"));
    email.set_subject("Email with Sender");
    email.add_body("Test Body");
    client.send(email);
    auto output = redirect.get_output();
    REQUIRE(output.find("From: from@example.com") != std::string::npos);
    REQUIRE(output.find("Sender: sender@example.com") != std::string::npos);
    REQUIRE(output.find("To: to@example.com") != std::string::npos);
  }
}
