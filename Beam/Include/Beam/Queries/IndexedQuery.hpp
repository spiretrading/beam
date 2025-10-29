#ifndef BEAM_INDEXED_QUERY_HPP
#define BEAM_INDEXED_QUERY_HPP
#include <ostream>
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {

  /**
   * Queries over a value used as an index.
   * @tparam T The type used as the index.
   */
  template<typename T>
  class IndexedQuery {
    public:

      /** The type used as the index. */
      using Index = T;

      /** Constructs an IndexedQuery with a default index. */
      IndexedQuery();

      /**
       * Constructs an IndexedQuery with a specified index.
       * @param index The index to use.
       */
      explicit IndexedQuery(Index index);

      /** Returns the index. */
      const Index& get_index() const;

      /** Sets the index. */
      void set_index(const Index& index);

    private:
      friend struct Shuttle<IndexedQuery>;
      Index m_index;
  };

  template<typename T>
  std::ostream& operator <<(std::ostream& out, const IndexedQuery<T>& query) {
    return out << query.get_index();
  }

  template<typename T>
  IndexedQuery<T>::IndexedQuery()
    : m_index() {}

  template<typename T>
  IndexedQuery<T>::IndexedQuery(Index index)
    : m_index(std::move(index)) {}

  template<typename T>
  const typename IndexedQuery<T>::Index& IndexedQuery<T>::get_index() const {
    return m_index;
  }

  template<typename T>
  void IndexedQuery<T>::set_index(const Index& index) {
    m_index = index;
  }

  template<typename T>
  struct Shuttle<IndexedQuery<T>> {
    template<IsShuttle S>
    void operator ()(
        S& shuttle, IndexedQuery<T>& value, unsigned int version) const {
      shuttle.shuttle("index", value.m_index);
    }
  };
}

#endif
