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

    /** Constructs a KeyValuePair. */
    KeyValuePair() = default;

    /**
     * Constructs a KeyValuePair.
     * @param key The pair's key.
     * @param value The value to associate with the <i>key</i>.
     */
    KeyValuePair(Key key, Value value);

    /** Tests if another pair has an equal key and value. */
    bool operator ==(const KeyValuePair& pair) const;

    /** Tests if another pair has a non-equal key or value. */
    bool operator !=(const KeyValuePair& pair) const;
  };

  template<typename Key, typename Value>
  std::ostream& operator <<(std::ostream& out,
      const KeyValuePair<Key, Value>& pair) {
    return out << '(' << pair.m_key << ' ' << pair.m_value << ')';
  }

  template<typename K, typename V>
  KeyValuePair<K, V>::KeyValuePair(Key key, Value value)
    : m_key(std::move(key)),
      m_value(std::move(value)) {}

  template<typename K, typename V>
  bool KeyValuePair<K, V>::operator ==(const KeyValuePair& pair) const {
    return m_key == pair.m_key && m_value == pair.m_value;
  }

  template<typename K, typename V>
  bool KeyValuePair<K, V>::operator !=(const KeyValuePair& pair) const {
    return !(*this == pair);
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
