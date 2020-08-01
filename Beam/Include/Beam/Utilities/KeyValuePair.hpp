#ifndef BEAM_KEY_VALUE_PAIR_HPP
#define BEAM_KEY_VALUE_PAIR_HPP
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
    using Key = KeyType;

    /** The value associated with the key. */
    using Value = ValueType;

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
  };

  template<typename KeyType, typename ValueType>
  KeyValuePair<KeyType, ValueType>::KeyValuePair(Key key, Value value)
    : m_key(std::move(key)),
      m_value(std::move(value)) {}
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
