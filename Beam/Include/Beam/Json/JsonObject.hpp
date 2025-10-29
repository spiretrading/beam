#ifndef BEAM_JSON_OBJECT_HPP
#define BEAM_JSON_OBJECT_HPP
#include <memory>
#include <ostream>
#include <string>
#include <unordered_map>
#include <boost/optional/optional.hpp>

namespace Beam {
  class JsonValue;

  /** Encapsulates an object represented using JSON. */
  class JsonObject {
    public:

      /** Constructs an empty JsonObject. */
      JsonObject() = default;

      /**
       * Returns a member of <code>this</code> JSON object, if the member has
       * not been initialized then it is added with a value of null.
       * @param name The name of the member to return.
       * @return The member with the specified <i>name</i>.
       */
      JsonValue& operator [](const std::string& name);

      /**
       * Returns a member of <code>this</code> JSON object.
       * @param name The name of the member to return.
       * @return The member with the specified <i>name</i>.
       */
      const JsonValue& at(const std::string& name) const;

      /**
       * Returns a member of <code>this</code> JSON object.
       * @param name The name of the member to return.
       * @return The member with the specified <i>name</i>.
       */
      boost::optional<const JsonValue&> get(const std::string& name) const;

      /**
       * Sets the value of a member.
       * @param name The name of the member to set.
       * @param value The value of the member.
       */
      void set(const std::string& name, const JsonValue& value);

      /**
       * Saves <code>this</code> object to an output stream.
       * @param sink The stream to save <code>this</code> object to.
       */
      void save(std::ostream& sink) const;

      /**
       * Returns <code>true</code> iff <code>this</code> JSON object is equal
       * to another.
       * @param object The JSON object to test for equality.
       * @return <code>true</code> iff <code>this</code> has exactly the same
       *         key/value pairs as <i>object</i>.
       */
      bool operator ==(const JsonObject& object) const;

      /**
       * Returns <code>true</code> iff <code>this</code> JSON object is not
       * equal to another.
       * @param object The JSON object to test for inequality.
       * @return <code>!(*this == object)</code>
       */
      bool operator !=(const JsonObject& object) const;

    private:
      std::unordered_map<std::string, std::shared_ptr<JsonValue>> m_members;
  };

  /**
   * Saves a JSON object to an output stream.
   * @param sink The stream to save the object to.
   * @param object The JSON object to save.
   * @return <code>sink</code>
   */
  inline std::ostream& operator <<(
      std::ostream& sink, const JsonObject& object) {
    object.save(sink);
    return sink;
  }
}

#include "Beam/Json/JsonValue.hpp"

namespace Beam {
  inline JsonValue& JsonObject::operator [](const std::string& name) {
    auto i = m_members.find(name);
    if(i == m_members.end()) {
      i = m_members.insert(
        std::pair(name, std::make_shared<JsonValue>())).first;
    }
    return *i->second;
  }

  inline const JsonValue& JsonObject::at(const std::string& name) const {
    return *m_members.at(name);
  }

  inline boost::optional<const JsonValue&> JsonObject::get(
      const std::string& name) const {
    auto i = m_members.find(name);
    if(i == m_members.end()) {
      return boost::none;
    }
    return *i->second;
  }

  inline void JsonObject::set(const std::string& name, const JsonValue& value) {
    auto& self = m_members[name];
    if(self) {
      *self = value;
    } else {
      self = std::make_shared<JsonValue>(value);
    }
  }

  inline void JsonObject::save(std::ostream& sink) const {
    sink << '{';
    auto is_first_member = true;
    for(auto& member : m_members) {
      if(!is_first_member) {
        sink << ',';
      }
      is_first_member = false;
      sink << '\"' << member.first << "\":";
      member.second->save(sink);
    }
    sink << '}';
  }

  inline bool JsonObject::operator ==(const JsonObject& object) const {
    if(m_members.size() != object.m_members.size()) {
      return false;
    }
    for(auto& member : m_members) {
      auto value = object.get(member.first);
      if(!value || !(*value == *member.second)) {
        return false;
      }
    }
    return true;
  }

  inline bool JsonObject::operator !=(const JsonObject& object) const {
    return !(*this == object);
  }
}

#endif
