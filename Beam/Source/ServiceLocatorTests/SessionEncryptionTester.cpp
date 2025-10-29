#include <unordered_set>
#include <doctest/doctest.h>
#include "Beam/ServiceLocator/SessionEncryption.hpp"

using namespace Beam;

TEST_SUITE("SessionEncryption") {
  TEST_CASE("generate_session_id") {
    auto session_id = generate_session_id();
    REQUIRE(session_id.length() == SESSION_ID_LENGTH);
    for(auto character : session_id) {
      REQUIRE(character >= 'a');
      REQUIRE(character <= 'z');
    }
  }

  TEST_CASE("generate_session_id_unique") {
    auto session_ids = std::unordered_set<std::string>();
    auto iterations = 1000;
    for(auto i = 0; i < iterations; ++i) {
      auto session_id = generate_session_id();
      REQUIRE(session_ids.find(session_id) == session_ids.end());
      session_ids.insert(session_id);
    }
    REQUIRE(session_ids.size() == iterations);
  }

  TEST_CASE("compute_sha_consistent") {
    auto source = std::string("test_string");
    auto hash1 = compute_sha(source);
    auto hash2 = compute_sha(source);
    REQUIRE(hash1 == hash2);
  }

  TEST_CASE("compute_sha_different") {
    auto source1 = std::string("test_string_1");
    auto source2 = std::string("test_string_2");
    auto hash1 = compute_sha(source1);
    auto hash2 = compute_sha(source2);
    REQUIRE(hash1 != hash2);
  }

  TEST_CASE("compute_sha_uppercase_hexadecimal") {
    auto source = std::string("test");
    auto hash = compute_sha(source);
    for(auto character : hash) {
      auto is_uppercase_hex = (character >= '0' && character <= '9') ||
        (character >= 'A' && character <= 'F');
      REQUIRE(is_uppercase_hex);
    }
  }

  TEST_CASE("compute_sha_correct_length") {
    auto source = std::string("test");
    auto hash = compute_sha(source);
    REQUIRE(hash.length() == 40);
  }

  TEST_CASE("compute_sha_handles_empty_string") {
    auto source = std::string();
    auto hash = compute_sha(source);
    REQUIRE(hash.length() == 40);
    REQUIRE(hash == "DA39A3EE5E6B4B0D3255BFEF95601890AFD80709");
  }

  TEST_CASE("compute_sha_handles_string_view") {
    auto source = std::string("test_string");
    auto view = std::string_view(source);
    auto hash1 = compute_sha(view);
    auto hash2 = compute_sha(source);
    REQUIRE(hash1 == hash2);
  }

  TEST_CASE("compute_sha_produces_known_hash_for_known_input") {
    auto source = std::string("hello");
    auto hash = compute_sha(source);
    REQUIRE(hash == "AAF4C61DDCC5E8A2DABEDE0F3B482CD9AEA9434D");
  }

  TEST_CASE("generate_encryption_key_produces_non_zero_values") {
    auto all_zero = true;
    auto iterations = 100;
    for(auto i = 0; i < iterations; ++i) {
      auto key = generate_encryption_key();
      if(key != 0) {
        all_zero = false;
        break;
      }
    }
    REQUIRE(!all_zero);
  }

  TEST_CASE("generate_encryption_key_produces_different_values") {
    auto keys = std::unordered_set<unsigned int>();
    auto iterations = 1000;
    for(auto i = 0; i < iterations; ++i) {
      auto key = generate_encryption_key();
      keys.insert(key);
    }
    REQUIRE(keys.size() > iterations / 2);
  }

  TEST_CASE("generate_encryption_key_produces_full_range_values") {
    auto has_high_bit = false;
    auto iterations = 1000;
    for(auto i = 0; i < iterations; ++i) {
      auto key = generate_encryption_key();
      if(key > (std::numeric_limits<unsigned int>::max() / 2)) {
        has_high_bit = true;
        break;
      }
    }
    REQUIRE(has_high_bit);
  }
}
