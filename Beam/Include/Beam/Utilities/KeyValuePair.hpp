#ifndef BEAM_KEY_VALUE_PAIR_HPP
#define BEAM_KEY_VALUE_PAIR_HPP
#include <ostream>
#include <utility>
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /**
   * Stores a key/value pair.
   * @param <K> The pair's key.
   * @param <V> The value associated with the key.
   */
  template<typename K, typename V>
  struct KeyValuePair {

    /** The pair's key. */
    using Key = K;

    /** The value associated with the key. */
    using Value = V;

    /** The pair's key. */
    Key m_key;

    /** The value associated with the key. */
    Value m_value;

    bool operator ==(const KeyValuePair& pair) const = default;
  };

  template<typename Key, typename Value>
  std::ostream& operator <<(std::ostream& out,
      const KeyValuePair<Key, Value>& pair) {
    return out << '(' << pair.m_key << ' ' << pair.m_value << ')';
  }
}

namespace Beam::Serialization {
  template<typename K, typename V>
  struct Shuttle<KeyValuePair<K, V>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, KeyValuePair<K, V>& value,
        unsigned int version) const {
      shuttle.Shuttle("key", value.m_key);
      shuttle.Shuttle("value", value.m_value);
    }
  };
}

#endif
