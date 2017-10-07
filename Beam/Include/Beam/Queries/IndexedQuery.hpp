#ifndef BEAM_INDEXEDQUERY_HPP
#define BEAM_INDEXEDQUERY_HPP
#include <ostream>
#include "Beam/Queries/Queries.hpp"
#include "Beam/Serialization/DataShuttle.hpp"

namespace Beam {
namespace Queries {

  /*! \class IndexedQuery
      \brief Queries over a value used as an index.
      \tparam T The type used as the index.
   */
  template<typename T>
  class IndexedQuery {
    public:

      //! The type used as the index.
      using Index = T;

      //! Constructs an IndexedQuery with a default index.
      IndexedQuery();

      //! Constructs an IndexedQuery with a specified index.
      /*!
        \param index The index to use.
      */
      IndexedQuery(const Index& index);

      //! Returns the index.
      const Index& GetIndex() const;

      //! Sets the index.
      void SetIndex(const Index& index);

    private:
      friend struct Serialization::Shuttle<IndexedQuery>;
      Index m_index;
  };

  template<typename T>
  std::ostream& operator <<(std::ostream& out, const IndexedQuery<T>& query) {
    return out << query.GetIndex();
  }

  template<typename T>
  IndexedQuery<T>::IndexedQuery()
      : m_index() {}

  template<typename T>
  IndexedQuery<T>::IndexedQuery(const Index& index)
      : m_index(index) {}

  template<typename T>
  const typename IndexedQuery<T>::Index& IndexedQuery<T>::GetIndex() const {
    return m_index;
  }

  template<typename T>
  void IndexedQuery<T>::SetIndex(const Index& index) {
    m_index = index;
  }
}
}

namespace Beam {
namespace Serialization {
  template<typename T>
  struct Shuttle<Queries::IndexedQuery<T>> {
    template<typename Shuttler>
    void operator ()(Shuttler& shuttle, Queries::IndexedQuery<T>& value,
        unsigned int version) {
      shuttle.Shuttle("index", value.m_index);
    }
  };
}
}

#endif
