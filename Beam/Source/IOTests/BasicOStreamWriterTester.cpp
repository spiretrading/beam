#include <sstream>
#include <doctest/doctest.h>
#include "Beam/IO/BasicOStreamWriter.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace Beam;

TEST_SUITE("BasicOStreamWriter") {
  TEST_CASE("write") {
    auto ss = std::stringstream();
    auto writer = BasicOStreamWriter(&ss);
    auto buffer = SharedBuffer("hello", 5);
    writer.write(buffer);
    REQUIRE(ss.str() == "hello");
  }

  TEST_CASE("write_wide") {
    auto ss = std::wostringstream();
    auto writer = BasicOStreamWriter(&ss);
    auto wide = std::wstring(L"hi");
    auto buffer = SharedBuffer(
      wide.c_str(), wide.size() * sizeof(std::wstring::value_type));
    writer.write(buffer);
    REQUIRE(ss.str() == L"hi");
  }
}
