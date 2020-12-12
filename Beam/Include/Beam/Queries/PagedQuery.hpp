#ifndef BEAM_PAGED_QUERY_HPP
#define BEAM_PAGED_QUERY_HPP
#include <ostream>
#include <boost/optional/optional.hpp>
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/IndexedQuery.hpp"
#include "Beam/Queries/Queries.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleOptional.hpp"

namespace Beam::Queries {

  /**
   * Composes various queries into a query whose results are returned in pages.
   * To use it, specify an optional anchor point around which results are
   * returned. For example to query for 20 city names that come after "Toronto",
   * set the anchor to "Toronto" and the snapshot limit to head 20. The anchor
   * can be left empty to query the first or last value in a data set.
   * @param <I> The type of index to query.
   * @param <A> The type of anchor to use.
   */
  template<typename I, typename A>
  class PagedQuery : public IndexedQuery<I>, public SnapshotLimitedQuery,
      public FilteredQuery {
    public:

      /** The type of anchor to use. */
      using Anchor = A;

      /** Returns the anchor. */
      const boost::optional<Anchor>& GetAnchor() const;

      /** Sets the anchor. */
      void SetAnchor(const boost::optional<Anchor>& anchor);

      /** Sets the anchor. */
      void SetAnchor(const Anchor& anchor);

    protected:
      template<typename Shuttler>
      void Shuttle(Shuttler& shuttle, unsigned int version);

    private:
      friend struct Serialization::DataShuttle;
      boost::optional<Anchor> m_anchor;
  };

  template<typename I, typename A>
  std::ostream& operator <<(std::ostream& out, const PagedQuery<I, A>& query) {
    out << '(' << query.GetIndex() << ' ' << query.GetSnapshotLimit() << ' ';
    if(query.GetAnchor()) {
      out << *query.GetAnchor() << ' ';
    }
    return out << query.GetFilter() << ')';
  }

  template<typename I, typename A>
  const boost::optional<typename PagedQuery<I, A>::Anchor>&
      PagedQuery<I, A>::GetAnchor() const {
    return m_anchor;
  }

  template<typename I, typename A>
  void PagedQuery<I, A>::SetAnchor(const boost::optional<Anchor>& anchor) {
    m_anchor = anchor;
  }

  template<typename I, typename A>
  void PagedQuery<I, A>::SetAnchor(const Anchor& anchor) {
    SetAnchor(boost::optional<Anchor>(anchor));
  }

  template<typename I, typename A>
  template<typename Shuttler>
  void PagedQuery<I, A>::Shuttle(Shuttler& shuttle, unsigned int version) {
    Beam::Serialization::Shuttle<IndexedQuery<I>>()(shuttle, *this, version);
    Beam::Serialization::Shuttle<SnapshotLimitedQuery>()(shuttle, *this,
      version);
    Beam::Serialization::Shuttle<FilteredQuery>()(shuttle, *this, version);
    shuttle.Shuttle("anchor", m_anchor);
  }
}

#endif
