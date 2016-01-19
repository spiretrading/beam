#ifndef BEAM_JSONOBJECT_HPP
#define BEAM_JSONOBJECT_HPP
#include <string>
#include <unordered_map>
#include <boost/optional/optional.hpp>
#include "Beam/Json/Json.hpp"

namespace Beam {

  /*! \class JsonObject
      \brief Encapsulates an object represented using JSON.
   */
  class JsonObject {
    public:

      //! Constructs an empty JsonObject.
      JsonObject();

      //! Returns a member of <code>this</code> JSON object, if the member has
      //! not been initialized then it is added with a value of null.
      /*!
        \param name The name of the member to return.
        \return The member with the specified <i>name</i>.
      */
      JsonValue& operator [](const std::string& name);

      //! Returns <code>true</code> iff <code>this</code> JSON object is equal
      //! to another.
      /*!
        \param object The JSON object to test for equality.
        \return <code>true</code> iff <code>this</code> has exactly the same
                key/value pairs as <i>object</i>.
      */
      bool operator ==(const JsonObject& object) const;

      //! Returns <code>true</code> iff <code>this</code> JSON object is not
      //! equal to another.
      /*!
        \param object The JSON object to test for inequality.
        \return <code>!(*this == object)</code>
      */
      bool operator !=(const JsonObject& object) const;

      //! Returns a member of <code>this</code> JSON object.
      /*!
        \param name The name of the member to return.
        \return The member with the specified <i>name</i>.
      */
      const JsonValue& At(const std::string& name) const;

      //! Returns a member of <code>this</code> JSON object.
      /*!
        \param name The name of the member to return.
        \return The member with the specified <i>name</i>.
      */
      boost::optional<const JsonValue&> Get(const std::string& name) const;

      //! Sets the value of a member.
      /*!
        \param name The name of the member to set.
        \param value The value of the member.
      */
      void Set(const std::string& name, const JsonValue& value);

      //! Saves <code>this</code> object to an output stream.
      /*!
        \param sink The stream to save <code>this</code> object to.
      */
      void Save(std::ostream& sink) const;

    private:
      std::unordered_map<std::string, std::shared_ptr<JsonValue>> m_members;
  };

  //! Saves a JSON object to an output stream.
  /*!
    \param sink The stream to save the object to.
    \param object The JSON object to save.
    \return <code>sink</code>
  */
  inline std::ostream& operator <<(std::ostream& sink,
      const JsonObject& object) {
    object.Save(sink);
    return sink;
  }
}

#include "Beam/Json/JsonValue.hpp"

namespace Beam {
  inline JsonObject::JsonObject() {}

  inline JsonValue& JsonObject::operator [](const std::string& name) {
    auto memberIterator = m_members.find(name);
    if(memberIterator == m_members.end()) {
      memberIterator = m_members.insert(
        std::make_pair(name, std::make_shared<JsonValue>())).first;
    }
    return *memberIterator->second;
  }

  inline bool JsonObject::operator ==(const JsonObject& object) const {
    if(m_members.size() != object.m_members.size()) {
      return false;
    }
    for(const auto& member : m_members) {
      boost::optional<const JsonValue&> value = object.Get(member.first);
      if(!value.is_initialized() || !(*value == *member.second)) {
        return false;
      }
    }
    return true;
  }

  inline bool JsonObject::operator !=(const JsonObject& object) const {
    return !(*this == object);
  }

  inline const JsonValue& JsonObject::At(const std::string& name) const {
    return *m_members.at(name);
  }

  inline boost::optional<const JsonValue&> JsonObject::Get(
      const std::string& name) const {
    auto memberIterator = m_members.find(name);
    if(memberIterator == m_members.end()) {
      return boost::optional<const JsonValue&>();
    }
    return *memberIterator->second;
  }

  inline void JsonObject::Set(const std::string& name, const JsonValue& value) {
    std::shared_ptr<JsonValue>& self = m_members[name];
    if(self == nullptr) {
      self = std::make_shared<JsonValue>(value);
    } else {
      *self = value;
    }
  }

  inline void JsonObject::Save(std::ostream& sink) const {
    sink << '{';
    bool isFirstMember = true;
    for(const auto& member : m_members) {
      if(!isFirstMember) {
        sink << ',';
      }
      isFirstMember = false;
      sink << '\"' << member.first << "\":";
      member.second->Save(sink);
    }
    sink << '}';
  }
}

#endif
