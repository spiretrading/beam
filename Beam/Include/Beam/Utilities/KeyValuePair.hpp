#ifndef BEAM_KEYVALUEPAIR_HPP
#define BEAM_KEYVALUEPAIR_HPP
#include <type_traits>
#include <utility>

namespace Beam {

  /*! \struct KeyValuePair
      \brief Stores a key/value pair.
      \tparam KeyType The pair's key.
      \tparam ValueType The value associated with the key.
   */
  template<typename KeyType, typename ValueType>
  struct KeyValuePair {

    //! The pair's key.
    using Key = KeyType;

    //! The value associated with the key.
    using Value = ValueType;

    //! The pair's key.
    Key m_key;

    //! The value associated with the key.
    Value m_value;

    //! Constructs a KeyValuePair.
    KeyValuePair() = default;

    //! Constructs a KeyValuePair.
    /*!
      \param key The pair's key.
      \param value The value to associate with the <i>key</i>.
    */
    KeyValuePair(Key key, Value value);
  };

  //! Builds a KeyValuePair.
  /*!
    \param key The pair's key.
    \param value The value to associate with the <i>key</i>.
  */
  template<typename Key, typename Value>
  KeyValuePair<std::decay_t<Key>, std::decay_t<Value>> MakeKeyValuePair(
      Key&& key, Value&& value) {
    return KeyValuePair<std::decay_t<Key>, std::decay_t<Value>>{
      std::forward<Key>(key), std::forward<Value>(value)};
  }

  template<typename KeyType, typename ValueType>
  KeyValuePair<KeyType, ValueType>::KeyValuePair(Key key, Value value)
      : m_key{std::move(key)},
        m_value{std::move(value)} {}
}

#endif
