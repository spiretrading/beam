#ifndef BEAM_SHUTTLE_JSON_VALUE_HPP
#define BEAM_SHUTTLE_JSON_VALUE_HPP
#include "Beam/Json/JsonObject.hpp"
#include "Beam/Serialization/ShuttlePair.hpp"
#include "Beam/Serialization/ShuttleVariant.hpp"
#include "Beam/Serialization/ShuttleVector.hpp"

namespace Beam {
namespace Details {
  using SerializedJsonValue =
    std::variant<std::string, JsonNull, bool, double,
      std::vector<std::pair<std::string, JsonValue>>, std::vector<JsonValue>>;
}

  template<>
  struct Shuttle<JsonNull> {
    template<IsShuttle S>
    void operator ()(S& shuttle, JsonNull& value, unsigned int version) const {}
  };

  template<>
  struct Send<JsonValue> {
    template<IsSender S>
    void operator ()(S& sender, const JsonValue& value,
        unsigned int version) const {
      auto serialized = visit(value,
        [&] (const JsonObject& object) {
          auto members = std::vector<std::pair<std::string, JsonValue>>();
          for(auto& [name, value] : object.m_members) {
            members.emplace_back(name, *value);
          }
          return Details::SerializedJsonValue(members);
        }, [&] (const auto& value) {
          return Details::SerializedJsonValue(value);
        });
      sender.shuttle("value", serialized);
    }
  };

  template<>
  struct Receive<JsonValue> {
    template<IsReceiver R>
    void operator ()(
        R& receiver, JsonValue& value, unsigned int version) const {
      auto serialized = Details::SerializedJsonValue();
      receiver.shuttle("value", serialized);
      value = visit(serialized,
        [] (const std::vector<std::pair<std::string, JsonValue>>& members) {
          auto object = JsonObject();
          for(auto& [name, value] : members) {
            object.set(name, value);
          }
          return JsonValue(object);
        }, [] (const auto& value) {
          return JsonValue(value);
        });
    }
  };
}

#endif
