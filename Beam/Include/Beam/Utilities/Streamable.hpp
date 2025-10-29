#ifndef BEAM_STREAMABLE_HPP
#define BEAM_STREAMABLE_HPP
#include <map>
#include <ostream>
#include <set>
#include <sstream>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/core/demangle.hpp>

namespace Beam {

  /** Interface for an object that can be output to a stream. */
  class Streamable {
    public:
      virtual ~Streamable() = default;

    protected:

      /**
       * Outputs a textual representation of this object to a stream.
       * @param out The stream to output the textual representation to.
       * @return <i>out</i>.
       */
      virtual std::ostream& to_stream(std::ostream& out) const;

    private:
      friend std::ostream& operator <<(
        std::ostream& out, const Streamable& object);
  };

  /** Adaptor type to enable unintrusive support for streaming operators. */
  template<typename T>
  struct Stream {
    const T* m_value;

    /**
     * Constructs a Stream.
     * @param value The value to stream.
     */
    explicit Stream(const T& value) noexcept;
  };

  /**
   * Converts a Streamable object to a string.
   * @param object The object to convert to a string.
   * @return The string representation of the object.
   */
  inline std::string to_string(const Streamable& object) {
    auto out = std::ostringstream();
    out << object;
    return out.str();
  }

  inline std::ostream& operator <<(
      std::ostream& out, const Streamable& object) {
    return object.to_stream(out);
  }

  inline std::ostream& Streamable::to_stream(std::ostream& out) const {
    return out << boost::core::demangle(typeid(*this).name());
  }

  template<typename T>
  Stream<T>::Stream(const T& value) noexcept
    : m_value(&value) {}

  template<typename T>
  std::ostream& operator <<(std::ostream& out, const Stream<T>& source) {
    return out << *source.m_value;
  }

  template<typename T, typename A>
  std::ostream& operator <<(
      std::ostream& out, const Stream<std::vector<T, A>>& source) {
    auto& value = *source.m_value;
    out << '[';
    for(auto i = value.begin(); i != value.end(); ++i) {
      out << Stream(*i);
      if(i != value.end() - 1) {
        out << ", ";
      }
    }
    return out << ']';
  }

  template<typename K, typename V>
  std::ostream& operator <<(
      std::ostream& out, const Stream<std::map<K, V>>& source) {
    auto& value = *source.m_value;
    out << '{';
    for(auto i = value.begin(); i != value.end(); ++i) {
      out << Stream(i->first) << ": " << Stream(i->second);
      if(i != std::prev(value.end())) {
        out << ", ";
      }
    }
    return out << '}';
  }

  template<typename K, typename V>
  std::ostream& operator <<(
      std::ostream& out, const Stream<std::unordered_map<K, V>>& source) {
    auto& value = *source.m_value;
    out << '{';
    for(auto i = value.begin(); i != value.end(); ++i) {
      out << Stream(i->first) << ": " << Stream(i->second);
      if(i != std::prev(value.end())) {
        out << ", ";
      }
    }
    return out << '}';
  }

  template<typename T>
  std::ostream& operator <<(
      std::ostream& out, const Stream<std::set<T>>& source) {
    auto& value = *source.m_value;
    out << '{';
    for(auto i = value.begin(); i != value.end(); ++i) {
      out << Stream(*i);
      if(i != std::prev(value.end())) {
        out << ", ";
      }
    }
    return out << '}';
  }

  template<typename T>
  std::ostream& operator <<(
      std::ostream& out, const Stream<std::unordered_set<T>>& source) {
    auto& value = *source.m_value;
    out << '{';
    for(auto i = value.begin(); i != value.end(); ++i) {
      out << Stream(*i);
      if(i != std::prev(value.end())) {
        out << ", ";
      }
    }
    return out << '}';
  }
}

#endif
