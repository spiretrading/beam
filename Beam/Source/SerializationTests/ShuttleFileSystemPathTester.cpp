#include <filesystem>
#include <doctest/doctest.h>
#include "Beam/Serialization/ShuttleFileSystemPath.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("ShuttleFileSystemPath") {
  TEST_CASE("empty_path") {
    test_round_trip_shuttle(std::filesystem::path());
  }

  TEST_CASE("simple_relative_path") {
    test_round_trip_shuttle(std::filesystem::path("hello"));
  }

  TEST_CASE("nested_relative_path") {
    test_round_trip_shuttle(std::filesystem::path("dir/subdir/file.txt"));
  }

  TEST_CASE("path_with_spaces_and_special_characters") {
    test_round_trip_shuttle(std::filesystem::path("my folder/fi le@#$%.txt"));
  }

  TEST_CASE("path_with_dot_segments") {
    test_round_trip_shuttle(std::filesystem::path("dir/./sub/../file"));
  }

  TEST_CASE("root_path") {
    test_round_trip_shuttle(std::filesystem::path("/"));
  }
}
