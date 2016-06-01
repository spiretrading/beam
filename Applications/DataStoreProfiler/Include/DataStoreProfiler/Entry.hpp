#ifndef BEAM_DATASTOREPROFILER_ENTRY_HPP
#define BEAM_DATASTOREPROFILER_ENTRY_HPP
#include <cstdint>
#include <string>
#include <boost/date_time/posix_time/ptime.hpp>

namespace Beam {

  /*! \struct Entry
      \brief Stores a dummy value used to profile data stores.
   */
  struct Entry {

    //! The index/name of the entry.
    std::string m_name;

    //! Item A.
    int m_itemA;

    //! Item B.
    std::int64_t m_itemB;

    //! Item C.
    std::int64_t m_itemC;

    //! Item D.
    std::string m_itemD;

    //! The timestamp.
    boost::posix_time::ptime m_timestamp;

    //! Constructs a default Entry.
    Entry() = default;

    //! Constructs an Entry.
    Entry(std::string name, int itemA, std::int64_t itemB, std::int64_t itemC,
      std::string itemD, boost::posix_time::ptime timestamp);
  };

  //! Returns the Entry's timestamp.
  inline boost::posix_time::ptime& GetTimestamp(Entry& entry) {
    return entry.m_timestamp;
  }

  //! Returns the Entry's timestamp.
  inline const boost::posix_time::ptime& GetTimestamp(const Entry& entry) {
    return entry.m_timestamp;
  }

  inline Entry::Entry(std::string name, int itemA, std::int64_t itemB,
      std::int64_t itemC, std::string itemD, boost::posix_time::ptime timestamp)
      : m_name{std::move(name)},
        m_itemA{itemA},
        m_itemB{itemB},
        m_itemC{itemC},
        m_itemD{itemD},
        m_timestamp{timestamp} {}
}

#endif
