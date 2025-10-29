#include <doctest/doctest.h>
#include "Beam/Codecs/SizeDeclarativeDecoder.hpp"
#include "Beam/CodecsTests/ReverseDecoder.hpp"
#include "Beam/IO/SharedBuffer.hpp"

using namespace boost;
using namespace boost::endian;
using namespace Beam;
using namespace Beam::Tests;

TEST_SUITE("SizeDeclarativeDecoder") {
  TEST_CASE("empty_decode") {
    auto decoder = SizeDeclarativeDecoder<ReverseDecoder>();
    auto message = from<SharedBuffer>("");
    auto decoded_message = from<SharedBuffer>("");
    auto output = SharedBuffer();
    append(output, native_to_big<std::uint32_t>(message.get_size()));
    append(output, message);
    auto decoded_buffer = SharedBuffer();
    auto decode_size = decoder.decode(output, out(decoded_buffer));
    auto expected_decode_size = decoded_message.get_size();
    REQUIRE(decode_size == expected_decode_size);
    REQUIRE(decoded_buffer == decoded_message);
  }

  TEST_CASE("decode") {
    auto decoder = SizeDeclarativeDecoder<ReverseDecoder>();
    auto message = from<SharedBuffer>("hello");
    auto decoded_message = from<SharedBuffer>("olleh");
    auto output = SharedBuffer();
    append(output, native_to_big<std::uint32_t>(message.get_size()));
    append(output, message);
    auto decoded_buffer = SharedBuffer();
    auto decode_size = decoder.decode(output, out(decoded_buffer));
    auto expected_decode_size = decoded_message.get_size();
    REQUIRE(decode_size == expected_decode_size);
    REQUIRE(decoded_buffer == decoded_message);
  }
}
