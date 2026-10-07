#ifndef BEAM_JSON_VALUE_HPP
#define BEAM_JSON_VALUE_HPP
#include <array>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "Beam/Utilities/OverloadSet.hpp"

namespace Beam {

  /** Dummy class used to represent a null value. */
  struct JsonNull {

    /** Always returns <code>true</code>. */
    bool operator ==(JsonNull rhs) const;
  };

namespace Details {
  inline void save_json_string(std::ostream& sink, const std::string& value) {
    sink.put('"');
    for(auto character : value) {
      switch(character) {
        case '"':
          sink << "\\\"";
          break;
        case '\\':
          sink << "\\\\";
          break;
        case '\b':
          sink << "\\b";
          break;
        case '\f':
          sink << "\\f";
          break;
        case '\n':
          sink << "\\n";
          break;
        case '\r':
          sink << "\\r";
          break;
        case '\t':
          sink << "\\t";
          break;
        default:
          auto byte = static_cast<unsigned char>(character);
          if(byte < 0x20) {
            auto digits = "0123456789abcdef";
            sink << "\\u00" << digits[byte >> 4] << digits[byte & 0xf];
          } else {
            sink.put(character);
          }
          break;
      }
    }
    sink.put('"');
  }

  using JsonVariant = std::variant<
    std::string, JsonNull, bool, double, JsonObject, std::vector<JsonValue>>;
}

  /**
   * Whether an integer is too wide to be represented exactly by a JSON number.
   * @tparam T The type to test.
   */
  template<typename T>
  constexpr auto is_wide_integer =
    std::is_integral_v<T> && sizeof(T) > sizeof(std::int32_t);

  /** Wraps a standard variant over all JSON types. */
  class JsonValue : public Details::JsonVariant {
    public:

      /** Constructs a null value. */
      JsonValue() noexcept;

      /**
       * Constructs a JsonValue from a variant type.
       * @param value The value to copy.
       */
      JsonValue(const Details::JsonVariant& variant);

      /**
       * Copies a JsonValue.
       * @param value The value to copy.
       */
      JsonValue(const JsonValue& value);

      JsonValue(JsonValue&&) noexcept = default;

      /**
       * Constructs a null value.
       * @param value The value to represent.
       */
      JsonValue(JsonNull value) noexcept;

      /**
       * Constructs a bool value.
       * @param value The value to represent.
       */
      JsonValue(bool value) noexcept;

      /**
       * Constructs an integer value.
       * @param value The value to represent.
       */
      JsonValue(int value) noexcept;

      /**
       * Constructs an integer value.
       * @param value The value to represent.
       */
      JsonValue(std::int64_t value) noexcept;

      /**
       * Constructs a number value.
       * @param value The value to represent.
       */
      JsonValue(double value) noexcept;

      /**
       * Constructs a string value.
       * @param value The value to represent.
       */
      JsonValue(const std::string& value);

      /**
       * Constructs a string value.
       * @param value The value to represent.
       */
      JsonValue(const char* value);

      /**
       * Constructs an object value.
       * @param value The value to represent.
       */
      JsonValue(const JsonObject& value);

      /**
       * Constructs an array value.
       * @param value The value to represent.
       */
      JsonValue(const std::vector<JsonValue>& value);

      /**
       * Saves <code>this</code> object to an output stream.
       * @param sink The stream to save <code>this</code> object to.
       */
      void save(std::ostream& sink) const;

      /**
       * Tests if two JSON values are equal.
       * @param value The value to test for equality.
       * @return <code>true</code> iff both values have the same type and value;
       */
      bool operator ==(const JsonValue& value) const;

      /**
       * Tests if two JSON values are not equal.
       * @param value The value to test for inequality.
       * @return <code>!(*this == value)</code>
       */
      bool operator !=(const JsonValue& value) const;

      /**
       * Assigns a generic JSON value.
       * @param value The value to represent.
       * @return <code>*this</code>
       */
      JsonValue& operator =(const JsonValue& value);

      JsonValue& operator =(JsonValue&&) noexcept = default;

      /**
       * Assigns a null value.
       * @param value The value to represent.
       * @return <code>*this</code>
       */
      JsonValue& operator =(JsonNull value);

      /**
       * Assigns a bool value.
       * @param value The value to represent.
       * @return <code>*this</code>
       */
      JsonValue& operator =(bool value);

      /**
       * Assigns an integer value.
       * @param value The value to represent.
       * @return <code>*this</code>
       */
      JsonValue& operator =(int value);

      /**
       * Assigns an integer value.
       * @param value The value to represent.
       * @return <code>*this</code>
       */
      JsonValue& operator =(std::int64_t value);

      /**
       * Assigns a number value.
       * @param value The value to represent.
       * @return <code>*this</code>
       */
      JsonValue& operator =(double value);

      /**
       * Assigns a string value.
       * @param value The value to represent.
       * @return <code>*this</code>
       */
      JsonValue& operator =(const std::string& value);

      /**
       * Assigns an object value.
       * @param value The value to represent.
       * @return <code>*this</code>
       */
      JsonValue& operator =(const JsonObject& value);

      /**
       * Assigns an array value.
       * @param value The value to represent.
       * @return <code>*this</code>
       */
      JsonValue& operator =(const std::vector<JsonValue>& value);
  };

  /**
   * Saves a JSON value to an output stream.
   * @param sink The stream to save the object to.
   * @param value The JSON value to save.
   * @return <code>sink</code>
   */
  inline std::ostream& operator <<(std::ostream& sink, const JsonValue& value) {
    value.save(sink);
    return sink;
  }

  inline bool JsonNull::operator ==(JsonNull rhs) const {
    return true;
  }

  inline JsonValue::JsonValue() noexcept
    : Details::JsonVariant(JsonNull()) {}

  inline JsonValue::JsonValue(const Details::JsonVariant& variant)
    : Details::JsonVariant(variant) {}

  inline JsonValue::JsonValue(const JsonValue& value)
    : Details::JsonVariant(static_cast<const Details::JsonVariant&>(value)) {}

  inline JsonValue::JsonValue(JsonNull value) noexcept
    : Details::JsonVariant(value) {}

  inline JsonValue::JsonValue(bool value) noexcept
    : Details::JsonVariant(value) {}

  inline JsonValue::JsonValue(int value) noexcept
    : Details::JsonVariant(static_cast<double>(value)) {}

  inline JsonValue::JsonValue(std::int64_t value) noexcept
    : Details::JsonVariant(static_cast<double>(value)) {}

  inline JsonValue::JsonValue(double value) noexcept
    : Details::JsonVariant(value) {}

  inline JsonValue::JsonValue(const std::string& value)
    : Details::JsonVariant(value) {}

  inline JsonValue::JsonValue(const char* value)
    : JsonValue(std::string(value)) {}

  inline JsonValue::JsonValue(const JsonObject& value)
    : Details::JsonVariant(value) {}

  inline JsonValue::JsonValue(const std::vector<JsonValue>& value)
    : Details::JsonVariant(value) {}

  inline void JsonValue::save(std::ostream& sink) const {
    visit(*this,
      [&] (JsonNull value) {
        sink << "null";
      },
      [&] (bool value) {
        if(value) {
          sink << "true";
        } else {
          sink << "false";
        }
      },
      [&] (double value) {
        if(!std::isfinite(value)) {
          throw std::invalid_argument("Non-finite JSON number.");
        }
        auto buffer =
          std::array<char, std::numeric_limits<double>::max_digits10 +
            std::numeric_limits<double>::max_exponent10 + 3>();
        auto result = std::to_chars(buffer.data(),
          buffer.data() + buffer.size(), value, std::chars_format::fixed);
        sink.write(buffer.data(), result.ptr - buffer.data());
      },
      [&] (const std::string& value) {
        Details::save_json_string(sink, value);
      },
      [&] (const JsonObject& value) {
        value.save(sink);
      },
      [&] (const std::vector<JsonValue>& value) {
        sink << '[';
        for(auto i = value.begin(); i != value.end(); ++i) {
          i->save(sink);
          if(i != value.end() - 1) {
            sink << ',';
          }
        }
        sink << ']';
      });
  }

  inline bool JsonValue::operator ==(const JsonValue& value) const {
    return static_cast<const Details::JsonVariant&>(*this) ==
      static_cast<const Details::JsonVariant&>(value);
  }

  inline bool JsonValue::operator !=(const JsonValue& value) const {
    return !(*this == value);
  }

  inline JsonValue& JsonValue::operator =(const JsonValue& value) {
    if(this == &value) {
      return *this;
    }
    Details::JsonVariant::operator =(
      static_cast<const Details::JsonVariant&>(value));
    return *this;
  }

  inline JsonValue& JsonValue::operator =(JsonNull value) {
    Details::JsonVariant::operator =(value);
    return *this;
  }

  inline JsonValue& JsonValue::operator =(bool value) {
    Details::JsonVariant::operator =(value);
    return *this;
  }

  inline JsonValue& JsonValue::operator =(int value) {
    Details::JsonVariant::operator =(static_cast<double>(value));
    return *this;
  }

  inline JsonValue& JsonValue::operator =(std::int64_t value) {
    Details::JsonVariant::operator =(static_cast<double>(value));
    return *this;
  }

  inline JsonValue& JsonValue::operator =(double value) {
    Details::JsonVariant::operator =(value);
    return *this;
  }

  inline JsonValue& JsonValue::operator =(const std::string& value) {
    Details::JsonVariant::operator =(value);
    return *this;
  }

  inline JsonValue& JsonValue::operator =(const JsonObject& value) {
    Details::JsonVariant::operator =(value);
    return *this;
  }

  inline JsonValue& JsonValue::operator =(const std::vector<JsonValue>& value) {
    Details::JsonVariant::operator =(value);
    return *this;
  }
}

#endif
