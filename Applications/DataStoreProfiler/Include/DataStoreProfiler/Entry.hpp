#ifndef BEAM_DATA_STORE_PROFILER_ENTRY_HPP
#define BEAM_DATA_STORE_PROFILER_ENTRY_HPP
#include <cstdint>
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>

namespace Beam {

  /** Stores a dummy value used to profile data stores. */
  struct Entry {

    /** The index/name of the entry. */
    std::string m_name;

    /** Item A. */
    int m_item_a;

    /** Item B. */
    std::int64_t m_item_b;

    /** Item C. */
    std::int64_t m_item_c;

    /** Item D. */
    std::string m_item_d;

    /** The timestamp. */
    boost::posix_time::ptime m_timestamp;
  };

  /** Returns the Entry's timestamp. */
  inline auto& get_timestamp(Entry& entry) {
    return entry.m_timestamp;
  }

  /** Returns the Entry's timestamp. */
  inline auto& get_timestamp(const Entry& entry) {
    return entry.m_timestamp;
  }
}

#endif
