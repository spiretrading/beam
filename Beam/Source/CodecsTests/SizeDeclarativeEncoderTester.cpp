#include <doctest/doctest.h>
#include "Beam/Codecs/SizeDeclarativeEncoder.hpp"
#include "Beam/CodecsTests/ReverseEncoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace boost;
using namespace boost::endian;
using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("SizeDeclarativeEncoder") {
  TEST_CASE("empty_encode") {
    auto encoder = SizeDeclarativeEncoder<ReverseEncoder>();
    auto message = from<SharedBuffer>("");
    auto expected_payload = from<SharedBuffer>("");
    auto expected_output = SharedBuffer();
    append(expected_output, native_to_big<std::uint32_t>(message.get_size()));
    append(expected_output, expected_payload);
    auto destination = SharedBuffer();
    auto encode_size = encoder.encode(message, out(destination));
    auto expected_encode_size = expected_output.get_size();
    REQUIRE(encode_size == expected_encode_size);
    REQUIRE(destination == expected_output);
  }

  TEST_CASE("encode") {
    auto encoder = SizeDeclarativeEncoder<ReverseEncoder>();
    auto message = from<SharedBuffer>("hello");
    auto expected_payload = from<SharedBuffer>("olleh");
    auto expected_output = SharedBuffer();
    append(expected_output, native_to_big<std::uint32_t>(message.get_size()));
    append(expected_output, expected_payload);
    auto destination = SharedBuffer();
    auto encode_size = encoder.encode(message, out(destination));
    auto expected_encode_size = expected_output.get_size();
    REQUIRE(encode_size == expected_encode_size);
    REQUIRE(destination == expected_output);
  }
}
