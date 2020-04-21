#ifndef BEAM_STREAMABLE_HPP
#define BEAM_STREAMABLE_HPP
#include <map>
#include <ostream>
#include <set>
#include <typeinfo>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/core/demangle.hpp>
#include "Beam/Utilities/Utilities.hpp"

namespace Beam {

  /*! \class Streamable
      \brief Interface for an object that can be output to a stream.
   */
  class Streamable {
    protected:

      //! Outputs a textual representation of this object to a stream.
      /*!
        \param out The stream to output the textual representation to.
        \return <i>out</i>.
      */
      virtual std::ostream& ToStream(std::ostream& out) const;

    private:
      friend std::ostream& operator <<(std::ostream& out,
        const Streamable& object);
  };

  //! Adaptor type to enable unintrusive support for streaming operators.
  template<typename T>
  struct Stream {
    const T* m_value;

    //! Constructs a Stream.
    /*!
      \param value The value to stream.
    */
    explicit Stream(const T& value);
  };

  inline std::ostream& operator <<(std::ostream& out,
      const Streamable& object) {
    return object.ToStream(out);
  }

  inline std::ostream& Streamable::ToStream(std::ostream& out) const {
    return out << boost::core::demangle(typeid(*this).name());
  }

  template<typename T>
  Stream<T>::Stream(const T& value)
    : m_value(&value) {}

  template<typename T, typename A>
  std::ostream& operator <<(std::ostream& out,
      const Stream<std::vector<T, A>>& source) {
    auto& value = *source.m_value;
    out << "[";
    for(auto i = value.begin(); i != value.end(); ++i) {
      out << *i;
      if(i != value.end() - 1) {
        out << ", ";
      }
    }
    out << "]";
    return out;
  }

  template<typename K, typename V>
  std::ostream& operator <<(std::ostream& out,
      const Stream<std::map<K, V>>& source) {
    auto& value = *source.m_value;
    out << "{";
    for(typename std::map<K, V>::const_iterator i = value.begin();
        i != value.end(); ++i) {
      out << i->first << ": " << i->second;
      if(i != value.end() - 1) {
        out << ", ";
      }
    }
    out << "}";
    return out;
  }

  template<typename T>
  std::ostream& operator <<(std::ostream& out,
      const Stream<std::set<T>>& source) {
    auto& value = *source.m_value;
    out << "{";
    for(typename std::set<T>::const_iterator i = value.begin();
        i != value.end(); ++i) {
      out << *i;
      if(i != value.end() - 1) {
        out << ", ";
      }
    }
    out << "}";
    return out;
  }
}

#endif
