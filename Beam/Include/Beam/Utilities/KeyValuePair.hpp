#ifndef BEAM_KEY_VALUE_PAIR_HPP
#define BEAM_KEY_VALUE_PAIR_HPP
#include <ostream>
#include <utility>
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /**
   * Stores a key/value pair.
   * @tparam K The pair's key.
   * @tparam V The value associated with the key.
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

  template<typename K, typename V>
  std::ostream& operator <<(std::ostream& out, const KeyValuePair<K, V>& pair) {
    return out << '(' << pair.m_key << ' ' << pair.m_value << ')';
  }

  template<typename K, typename V>
  struct Shuttle<KeyValuePair<K, V>> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, KeyValuePair<K, V>& value, unsigned int version) const {
      shuttle.shuttle("key", value.m_key);
      shuttle.shuttle("value", value.m_value);
    }
  };
}

#endif
