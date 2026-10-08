#include <cstring>
#include <limits>
#include <stdexcept>
#include "Beam/Json/JsonParser.hpp"
#include "Beam/SerializationTests/ValueShuttleTests.hpp"

using namespace Beam;
using namespace Beam::Tests;

namespace {
  enum OrdinaryDefault { ORDINARY_NEGATIVE = -3, ORDINARY_POSITIVE = 27 };

  enum PlainDefault : std::int32_t {
    NEGATIVE = -7,
    POSITIVE = 42
  };

  enum class SignedDefault : std::int16_t { NEGATIVE = -42 };

  enum class ScopedDefault : std::uint64_t {
    WIDE = 0x100000007
  };

  enum PlainSeparate { PLAIN_SEPARATE_READY = 17 };
  enum class ScopedSeparate { READY = 18 };
  enum PlainShuttle { PLAIN_SHUTTLE_READY = 19 };
  enum class ScopedShuttle { READY = 20 };
}

namespace Beam {
  template<>
  struct Send<PlainSeparate> {
    template<IsSender S>
    void operator ()(S& sender, const char* name, PlainSeparate value) const {
      if(value != PLAIN_SEPARATE_READY) {
        throw std::invalid_argument("Invalid state.");
      }
      sender.send(name, std::string("READY"));
    }
  };

  template<>
  struct Receive<PlainSeparate> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name, PlainSeparate& value) const {
      if(receive<std::string>(receiver, name) != "READY") {
        throw std::invalid_argument("Invalid state.");
      }
      value = PLAIN_SEPARATE_READY;
    }
  };

  template<>
  struct Send<ScopedSeparate> {
    template<IsSender S>
    void operator ()(S& sender, const char* name, ScopedSeparate value) const {
      if(value != ScopedSeparate::READY) {
        throw std::invalid_argument("Invalid state.");
      }
      sender.send(name, std::string("READY"));
    }
  };

  template<>
  struct Receive<ScopedSeparate> {
    template<IsReceiver R>
    void operator ()(R& receiver, const char* name, ScopedSeparate& value) const {
      if(receive<std::string>(receiver, name) != "READY") {
        throw std::invalid_argument("Invalid state.");
      }
      value = ScopedSeparate::READY;
    }
  };

  template<>
  struct Shuttle<PlainShuttle> {
    template<IsShuttle S>
    void operator ()(S& shuttle, const char* name, PlainShuttle& value) const {
      if constexpr(IsReceiver<S>) {
        auto text = std::string();
        shuttle.shuttle(name, text);
        if(text != "READY") {
          throw std::invalid_argument("Invalid state.");
        }
        value = PLAIN_SHUTTLE_READY;
      } else {
        if(value != PLAIN_SHUTTLE_READY) {
          throw std::invalid_argument("Invalid state.");
        }
        shuttle.shuttle(name, std::string("READY"));
      }
    }
  };

  template<>
  struct Shuttle<ScopedShuttle> {
    template<IsShuttle S>
    void operator ()(S& shuttle, const char* name, ScopedShuttle& value) const {
      if constexpr(IsReceiver<S>) {
        auto text = std::string();
        shuttle.shuttle(name, text);
        if(text != "READY") {
          throw std::invalid_argument("Invalid state.");
        }
        value = ScopedShuttle::READY;
      } else {
        if(value != ScopedShuttle::READY) {
          throw std::invalid_argument("Invalid state.");
        }
        shuttle.shuttle(name, std::string("READY"));
      }
    }
  };

  template<>
  struct Shuttle<ScopedSeparate> {
    template<IsShuttle S>
    void operator ()(S&, const char*, ScopedSeparate&) const {
      throw std::logic_error("Send and Receive must take precedence.");
    }
  };
}

namespace {
  struct EnumFields {
    PlainDefault m_plain = NEGATIVE;
    ScopedDefault m_scoped = ScopedDefault::WIDE;
    PlainSeparate m_plain_separate = PLAIN_SEPARATE_READY;
    ScopedSeparate m_scoped_separate = ScopedSeparate::READY;
    PlainShuttle m_plain_shuttle = PLAIN_SHUTTLE_READY;
    ScopedShuttle m_scoped_shuttle = ScopedShuttle::READY;

    template<IsShuttle S>
    void shuttle(S& shuttle, unsigned int version) {
      shuttle.shuttle("plain", m_plain);
      shuttle.shuttle("scoped", m_scoped);
      shuttle.shuttle("plain_separate", m_plain_separate);
      shuttle.shuttle("scoped_separate", m_scoped_separate);
      shuttle.shuttle("plain_shuttle", m_plain_shuttle);
      shuttle.shuttle("scoped_shuttle", m_scoped_shuttle);
    }
  };

  void test_default(auto value) {
    using Enum = decltype(value);
    auto base = static_cast<std::int32_t>(value);
    REQUIRE(to_json(value) == to_json(base));
    REQUIRE(from_json<Enum>(to_json(base)) == static_cast<Enum>(base));
    auto sender = BinarySender<SharedBuffer>();
    auto expected = encode<SharedBuffer>(sender, base);
    auto actual = encode<SharedBuffer>(sender, value);
    REQUIRE(actual.get_size() == sizeof(std::int32_t));
    REQUIRE(std::memcmp(actual.get_data(), expected.get_data(),
      expected.get_size()) == 0);
    auto receiver = BinaryReceiver<SharedBuffer>();
    receiver.set(Ref(expected));
    REQUIRE(receive<Enum>(receiver) == static_cast<Enum>(base));
  }

  void test_named(auto value) {
    using Enum = decltype(value);
    REQUIRE(to_json(value) == R"("READY")");
    REQUIRE(from_json<Enum>(std::string(R"("READY")")) == value);
    REQUIRE_THROWS_AS(from_json<Enum>(std::string(R"("OTHER")")),
      std::invalid_argument);
    REQUIRE_THROWS_AS(from_json<Enum>(std::string("17")),
      SerializationException);
    auto sender = BinarySender<SharedBuffer>();
    auto expected = encode<SharedBuffer>(sender, std::string("READY"));
    auto actual = encode<SharedBuffer>(sender, value);
    REQUIRE(actual.get_size() == expected.get_size());
    REQUIRE(std::memcmp(actual.get_data(), expected.get_data(),
      expected.get_size()) == 0);
    auto receiver = BinaryReceiver<SharedBuffer>();
    receiver.set(Ref(expected));
    REQUIRE(receive<Enum>(receiver) == value);
  }
}

TEST_SUITE("ShuttleEnum") {
  TEST_CASE("default_format_is_unchanged") {
    test_default(ORDINARY_NEGATIVE);
    test_default(ORDINARY_POSITIVE);
    test_default(NEGATIVE);
    test_default(POSITIVE);
    test_default(static_cast<PlainDefault>(std::numeric_limits<int>::min()));
    test_default(static_cast<PlainDefault>(std::numeric_limits<int>::max()));
    test_default(SignedDefault::NEGATIVE);
    test_default(ScopedDefault::WIDE);
    test_default(static_cast<ScopedDefault>(42));
  }

  TEST_CASE("custom_send_receive_and_shuttle") {
    test_named(PLAIN_SEPARATE_READY);
    test_named(ScopedSeparate::READY);
    test_named(PLAIN_SHUTTLE_READY);
    test_named(ScopedShuttle::READY);
    auto fields = EnumFields();
    auto expected = parse<JsonValue>(to_json(fields));
    test_round_trip_shuttle(fields, [&] (const auto& received) {
      REQUIRE(parse<JsonValue>(to_json(received)) == expected);
    });
    auto received = from_json<EnumFields>(to_json(fields));
    REQUIRE(parse<JsonValue>(to_json(received)) == expected);
    auto& object = get<JsonObject>(expected);
    REQUIRE(object.at("plain") == -7);
    REQUIRE(object.at("scoped") == 7);
    for(auto& name : {"plain_separate", "scoped_separate", "plain_shuttle",
        "scoped_shuttle"}) {
      REQUIRE(object.at(name) == JsonValue("READY"));
    }
  }
}
