#include <doctest/doctest.h>
#include "Beam/Utilities/Bcrypt.hpp"

using namespace Beam;

TEST_SUITE("Bcrypt") {
  TEST_CASE("hash_simple_password") {
    auto password = std::string("test_password");
    auto hash = bcrypt(password);
    REQUIRE(!hash.empty());
    REQUIRE(hash.size() == 60);
    REQUIRE(hash[0] == '$');
    REQUIRE(hash[1] == '2');
  }

  TEST_CASE("hash_with_default_rounds") {
    auto password = std::string("password123");
    auto hash = bcrypt(password);
    REQUIRE(hash.find("$2a$10$") == 0);
  }

  TEST_CASE("hash_with_minimum_rounds") {
    auto password = std::string("test");
    auto hash = bcrypt(password, 4);
    REQUIRE(hash.find("$2a$04$") == 0);
  }

  TEST_CASE("hash_empty_password") {
    auto password = std::string("");
    auto hash = bcrypt(password);
    REQUIRE(!hash.empty());
    REQUIRE(hash.size() == 60);
  }

  TEST_CASE("hash_long_password") {
    auto password = std::string(200, 'a');
    auto hash = bcrypt(password);
    REQUIRE(!hash.empty());
    REQUIRE(hash.size() == 60);
  }

  TEST_CASE("hash_special_characters") {
    auto password = std::string("p@$$w0rd!#%&*()");
    auto hash = bcrypt(password);
    REQUIRE(!hash.empty());
    REQUIRE(hash.size() == 60);
  }

  TEST_CASE("same_password_different_hashes") {
    auto password = std::string("same_password");
    auto hash1 = bcrypt(password);
    auto hash2 = bcrypt(password);
    REQUIRE(hash1 != hash2);
  }

  TEST_CASE("matches_correct_password") {
    auto password = std::string("correct_password");
    auto hash = bcrypt(password);
    REQUIRE(bcrypt_matches(password, hash));
  }

  TEST_CASE("matches_incorrect_password") {
    auto password = std::string("correct_password");
    auto hash = bcrypt(password);
    auto wrong_password = std::string("wrong_password");
    REQUIRE(!bcrypt_matches(wrong_password, hash));
  }

  TEST_CASE("matches_empty_password") {
    auto password = std::string("");
    auto hash = bcrypt(password);
    REQUIRE(bcrypt_matches("", hash));
    REQUIRE(!bcrypt_matches("not_empty", hash));
  }

  TEST_CASE("matches_empty_hash") {
    auto password = std::string("password");
    auto empty_hash = std::string("");
    REQUIRE(!bcrypt_matches(password, empty_hash));
  }

  TEST_CASE("matches_invalid_hash_format") {
    auto password = std::string("password");
    auto invalid_hash = std::string("not_a_bcrypt_hash");
    REQUIRE(!bcrypt_matches(password, invalid_hash));
  }

  TEST_CASE("matches_case_sensitive") {
    auto password = std::string("Password");
    auto hash = bcrypt(password);
    REQUIRE(bcrypt_matches("Password", hash));
    REQUIRE(!bcrypt_matches("password", hash));
    REQUIRE(!bcrypt_matches("PASSWORD", hash));
  }

  TEST_CASE("matches_special_characters") {
    auto password = std::string("p@$$w0rd!#%");
    auto hash = bcrypt(password);
    REQUIRE(bcrypt_matches(password, hash));
    REQUIRE(!bcrypt_matches("p@ssw0rd!#%", hash));
  }

  TEST_CASE("matches_whitespace_sensitive") {
    auto password = std::string("password with spaces");
    auto hash = bcrypt(password);
    REQUIRE(bcrypt_matches("password with spaces", hash));
    REQUIRE(!bcrypt_matches("passwordwithspaces", hash));
    REQUIRE(!bcrypt_matches("password  with  spaces", hash));
  }
}
