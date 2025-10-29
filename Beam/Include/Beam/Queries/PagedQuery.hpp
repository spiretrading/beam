#ifndef BEAM_PAGED_QUERY_HPP
#define BEAM_PAGED_QUERY_HPP
#include <ostream>
#include <boost/optional/optional.hpp>
#include "Beam/Queries/FilteredQuery.hpp"
#include "Beam/Queries/IndexedQuery.hpp"
#include "Beam/Queries/SnapshotLimitedQuery.hpp"
#include "Beam/Serialization/DataShuttle.hpp"
#include "Beam/Serialization/ShuttleOptional.hpp"

namespace Beam {

  /**
   * Composes various queries into a query whose results are returned in pages.
   * To use it, specify an optional anchor point around which results are
   * returned. For example to query for 20 city names that come after "Toronto",
   * set the anchor to "Toronto" and the snapshot limit to head 20. The anchor
   * can be left empty to query the first or last value in a data set.
   * @tparam I The type of index to query.
   * @tparam A The type of anchor to use.
   */
  template<typename I, typename A>
  class PagedQuery : public IndexedQuery<I>, public SnapshotLimitedQuery,
      public FilteredQuery {
    public:

      /** The type of anchor to use. */
      using Anchor = A;

      /** Returns the anchor. */
      const boost::optional<Anchor>& get_anchor() const;

      /** Sets the anchor. */
      void set_anchor(const boost::optional<Anchor>& anchor);

      /** Sets the anchor. */
      void set_anchor(const Anchor& anchor);

    protected:
      template<IsShuttle S>
      void shuttle(S& shuttle, unsigned int version);

    private:
      friend struct DataShuttle;
      boost::optional<Anchor> m_anchor;
  };

  template<typename I, typename A>
  std::ostream& operator <<(std::ostream& out, const PagedQuery<I, A>& query) {
    out << '(' << query.get_index() << ' ' << query.get_snapshot_limit() << ' ';
    if(query.get_anchor()) {
      out << *query.get_anchor() << ' ';
    }
    return out << query.get_filter() << ')';
  }

  template<typename I, typename A>
  const boost::optional<typename PagedQuery<I, A>::Anchor>&
      PagedQuery<I, A>::get_anchor() const {
    return m_anchor;
  }

  template<typename I, typename A>
  void PagedQuery<I, A>::set_anchor(const boost::optional<Anchor>& anchor) {
    m_anchor = anchor;
  }

  template<typename I, typename A>
  void PagedQuery<I, A>::set_anchor(const Anchor& anchor) {
    set_anchor(boost::optional<Anchor>(anchor));
  }

  template<typename I, typename A>
  template<IsShuttle S>
  void PagedQuery<I, A>::shuttle(S& shuttle, unsigned int version) {
    Beam::Shuttle<IndexedQuery<I>>()(shuttle, *this, version);
    Beam::Shuttle<SnapshotLimitedQuery>()(shuttle, *this, version);
    Beam::Shuttle<FilteredQuery>()(shuttle, *this, version);
    shuttle.shuttle("anchor", m_anchor);
  }
}

#endif
